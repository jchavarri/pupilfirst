open SchoolCustomize__Types;

[%%mel.raw {js|import "./SchoolCustomize__LinksEditor.css"|js}];

let str = React.string;

let t = I18n.t(~scope="components.SchoolCustomize__LinkEditor");
let ts = I18n.t(~scope="shared");

type nonrec kind = SchoolCustomize__LinkComponent.kind;

type nonrec state = {
  kind,
  title: string,
  url: string,
  titleInvalid: bool,
  urlInvalid: bool,
  formDirty: bool,
  adding: bool,
  deleting: array(Customizations.linkId),
};

type nonrec action =
  | UpdateKind(kind)
  | UpdateTitle(string, bool)
  | UpdateUrl(string, bool)
  | DisableForm
  | EnableForm
  | ClearForm
  | DisableDelete(Customizations.linkId);

let handleKindChange = (send, kind, event) => {
  event->React.Event.Mouse.preventDefault;
  send(UpdateKind(kind));
};

let isTitleInvalid = title => title->String.trim->String.length == 0;

let handleTitleChange = (send, event) => {
  let title = React.Event.Form.target(event)##value;
  switch (title) {
  | Some(value) => send(UpdateTitle(title, isTitleInvalid(value)))
  | None => ()
  };
};

let handleUrlChange = (send, event) => {
  let url = React.Event.Form.target(event)##value;
  send(UpdateUrl(url, url |> UrlUtils.isInvalid(false, _)));
};

module DestroySchoolLinkQuery = [%graphql
  {js|
  mutation DestroySchoolLinkMutation($id: ID!) {
    destroySchoolLink(id: $id) {
      success
    }
  }
  |js}
];

let handleDelete = (state, send, removeLinkCB, id, event) => {
  event->React.Event.Mouse.preventDefault;

  if (state.deleting->(Js.Array.includes(~value=id))) {
    ();
  } else {
    send(DisableDelete(id));

    DestroySchoolLinkQuery.make({id: id})
    |> Js.Promise.then_(_response => {
         removeLinkCB(id);
         Js.Promise.resolve();
       })
    |> ignore;
  };
};

let deleteIconClasses = deleting =>
  if (deleting) {"fas fa-spinner fa-pulse"} else {"far fa-trash-alt"};

let titleInputVisible = state =>
  switch (state.kind) {
  | HeaderLink
  | FooterLink => true
  | SocialLink => false
  };

let kindClasses = selected => {
  let classes = "nav-tab-item border-t cursor-pointer w-1/3 appearance-none flex justify-center items-center w-full text-sm text-center text-gray-800 bg-white hover:bg-gray-50 hover:text-gray-900 py-3 px-4 font-semibold leading-tight focus:outline-none focus:bg-gray-50 focus:text-gray-900";
  classes
  ++ (
    if (selected) {
      " nav-tab-item--selected text-primary-500 bg-white hover:bg-white hover:text-primary-500"
    } else {
      " text-gray-600"
    }
  );
};

let addLinkText = adding =>
  if (adding) {
    t("adding_new_link");
  } else {
    t("add_new_link");
  };

let addLinkDisabled = (state: state) =>
  if (state.adding) {
    true;
  } else if (state.formDirty) {
    switch (state.kind) {
    | HeaderLink
    | FooterLink =>
      isTitleInvalid(state.title) || state.url |> UrlUtils.isInvalid(false)
    | SocialLink => state.url |> UrlUtils.isInvalid(false)
    };
  } else {
    true;
  };

module CreateSchoolLinkQuery = [%graphql
  {js|
  mutation CreateSchoolLinkMutation($kind: String!, $title: String, $url: String!) {
    createSchoolLink(kind: $kind, title: $title, url: $url) @bsVariant {
      schoolLink {
        id
      }
    }
  }
|js}
];

let displayNewLink = (state: state, addLinkCB, id) =>
  (
    switch (state.kind) {
    | HeaderLink => Customizations.HeaderLink(id, state.title, state.url, 0)
    | FooterLink => Customizations.FooterLink(id, state.title, state.url, 0)
    | SocialLink => Customizations.SocialLink(id, state.url, 0)
    }
  )
  ->addLinkCB;

module CreateLinkError = {
  type nonrec t = [
    | `InvalidUrl
    | `InvalidLengthTitle
    | `InvalidKind
    | `BlankTitle
  ];

  let notification = error =>
    switch (error) {
    | `InvalidUrl => (t("error_head"), t("error_body"))
    | `InvalidKind => ("InvalidKind", "")
    | `InvalidLengthTitle => ("InvalidLengthTitle", "")
    | `BlankTitle => ("BlankTitle", "")
    };
};

module CreateLinkErrorHandler = GraphqlErrorHandler.Make(CreateLinkError);

let handleAddLink = (state, send, addLinkCB, event) => {
  event->React.Event.Mouse.preventDefault;

  if (addLinkDisabled(state)) {
    ();
  } else {
    send(DisableForm);
    (
      switch (state.kind) {
      | HeaderLink =>
        CreateSchoolLinkQuery.make({
          kind: "header",
          title: Some(state.title),
          url: state.url,
        })
      | FooterLink =>
        CreateSchoolLinkQuery.make({
          kind: "footer",
          title: Some(state.title),
          url: state.url,
        })
      | SocialLink =>
        CreateSchoolLinkQuery.make({
          kind: "social",
          title: None,
          url: state.url,
        })
      }
    )
    |> Js.Promise.then_(response =>
         switch (response##createSchoolLink) {
         | `SchoolLink(schoolLink) =>
           schoolLink##id |> displayNewLink(state, addLinkCB);
           send(ClearForm);
           Js.Promise.resolve();
         | `Errors(errors) =>
           Js.Promise.reject(CreateLinkErrorHandler.Errors(errors))
         }
       )
    |> CreateLinkErrorHandler.catch(() => send(EnableForm))
    |> ignore;
  };
};

let linksTitle = kind =>
  (
    switch (kind) {
    | SchoolCustomize__LinkComponent.HeaderLink => t("header_links")
    | FooterLink => t("sitemap_links")
    | SocialLink => t("social_links")
    }
  )
  ->str;

let unpackLinks = (kind, customizations) =>
  (
    switch (kind) {
    | SchoolCustomize__LinkComponent.HeaderLink =>
      Customizations.filterLinks(~header=true, customizations)
    | FooterLink => Customizations.filterLinks(~footer=true, customizations)
    | SocialLink => Customizations.filterLinks(~social=true, customizations)
    }
  )
  ->Customizations.unpackLinks;

let initialState = kind => {
  kind,
  title: "",
  url: "",
  titleInvalid: false,
  urlInvalid: false,
  formDirty: false,
  adding: false,
  deleting: [||],
};

let reducer = (state, action) =>
  switch (action) {
  | UpdateKind(kind) => {...state, kind, formDirty: true}
  | UpdateTitle(title, invalid) => {
      ...state,
      title,
      titleInvalid: invalid,
      formDirty: true,
    }
  | UpdateUrl(url, invalid) => {
      ...state,
      url,
      urlInvalid: invalid,
      formDirty: true,
    }
  | DisableForm => {...state, adding: true}
  | EnableForm => {...state, adding: false}
  | ClearForm => {...state, adding: false, title: "", url: ""}
  | DisableDelete(linkId) => {
      ...state,
      deleting:
        state.deleting->Js.Array.copy->(Js.Array.concat(~other=[|linkId|])),
    }
  };

let showLinks =
    (
      deleting,
      disableDeleteCB,
      removeLinkCB,
      updateLinkCB,
      moveLinkCB,
      kind,
      links,
    ) =>
  switch (links) {
  | [||] =>
    <div
      className="border border-gray-400 rounded italic text-gray-600 text-xs cursor-default mt-2 p-3">
      "There are no custom links here. Add some?"->str
    </div>
  | links =>
    links
    ->(
        Js.Array.mapi(~f=((id, title, url, _), index) =>
          <SchoolCustomize__LinkComponent
            key=id
            id
            title
            url
            kind
            removeLinkCB
            updateLinkCB
            moveLinkCB
            disableDeleteCB
            deleting
            index
            total={links->Js.Array.length}
          />
        )
      )
    ->React.array
  };

[@react.component]
let make =
    (
      ~kind,
      ~customizations,
      ~addLinkCB,
      ~moveLinkCB,
      ~removeLinkCB,
      ~updateLinkCB,
    ) => {
  let (state, send) = React.useReducer(reducer, initialState(kind));

  let disableDeleteCB = id => send(DisableDelete(id));

  <div className="mt-8 mx-8 pb-6">
    <h5 className="uppercase text-center border-b border-gray-300 pb-2">
      {t("manage_links")->str}
    </h5>
    <div className="mt-3">
      <label className="inline-block tracking-wide text-xs font-semibold">
        {t("location_link")->str}
      </label>
      <div
        role="tablist"
        className="flex bg-white border border-t-0 rounded-t mt-2">
        <button
          role="tab"
          ariaSelected={state.kind == HeaderLink}
          ariaLabel={t("show_header_title")}
          title={t("show_header_title")}
          className={kindClasses(state.kind == HeaderLink)}
          onClick={handleKindChange(send, HeaderLink)}>
          "Header"->str
        </button>
        <button
          role="tab"
          ariaSelected={state.kind == FooterLink}
          ariaLabel={t("footer_link_title")}
          title={t("footer_link_title")}
          className={kindClasses(state.kind == FooterLink) ++ " border-l"}
          onClick={handleKindChange(send, FooterLink)}>
          "Footer Sitemap"->str
        </button>
        <button
          role="tab"
          ariaSelected={state.kind == SocialLink}
          ariaLabel={t("social_links_title")}
          title={t("social_links_title")}
          className={kindClasses(state.kind == SocialLink) ++ " border-l"}
          onClick={handleKindChange(send, SocialLink)}>
          "Social"->str
        </button>
      </div>
    </div>
    <div className="p-5 border border-t-0 rounded-b">
      <label className="inline-block tracking-wide text-xs font-semibold mt-4">
        {linksTitle(state.kind)}
      </label>
      {showLinks(
         state.deleting,
         disableDeleteCB,
         removeLinkCB,
         updateLinkCB,
         moveLinkCB,
         state.kind,
         unpackLinks(state.kind, customizations),
       )}
      <DisablingCover disabled={state.adding}>
        <div className="flex mt-3" key="sc-links-editor__form-body">
          {if (state->titleInputVisible) {
             <div className="grow mr-4">
               <label
                 className="inline-block tracking-wide text-xs font-semibold"
                 htmlFor="link-title">
                 {t("title")->str}
               </label>
               <input
                 autoFocus=true
                 className="appearance-none block w-full bg-white border border-gray-300 rounded py-3 px-4 mt-2 leading-tight focus:outline-none focus:bg-white focus:ring-2 focus:ring-focusColor-500"
                 id="link-title"
                 type_="text"
                 placeholder={t("title_placeholder")}
                 onChange={handleTitleChange(send)}
                 value={state.title}
                 maxLength=24
               />
               <School__InputGroupError
                 message={t("cant_empty_message")}
                 active={state.titleInvalid}
               />
             </div>;
           } else {
             React.null;
           }}
          <div className="grow">
            <label
              className="inline-block tracking-wide text-xs font-semibold"
              htmlFor="link-full-url">
              {t("full_url")->str}
            </label>
            <input
              className="appearance-none block w-full bg-white border border-gray-300 rounded py-3 px-4 mt-2 leading-tight focus:outline-none focus:bg-white focus:ring-2 focus:ring-focusColor-500"
              id="link-full-url"
              type_="text"
              placeholder={t("full_url_placeholder")}
              onChange={handleUrlChange(send)}
              value={state.url}
            />
            <School__InputGroupError
              message={t("full_url_error")}
              active={state.urlInvalid}
            />
          </div>
        </div>
        <div className="flex justify-end">
          <button
            key="sc-links-editor__form-button"
            disabled={addLinkDisabled(state)}
            onClick={handleAddLink(state, send, addLinkCB)}
            className="btn btn-primary btn-large mt-6">
            {state.adding->addLinkText->str}
          </button>
        </div>
      </DisablingCover>
    </div>
  </div>;
};
