exception InvalidModeForPreview;
[%%raw {js|import "./MarkdownEditor.css"|js}];
[@mel.scope ("window", "pupilfirst")]
external maxUploadFileSize: int = "maxUploadFileSize";
let str = React.string;
let t = I18n.t(~scope="components.MarkdownEditor");
type nonrec fullscreenMode = [ | `Editor | `Preview | `Split];
type nonrec windowedMode = [ | `Editor | `Preview];
type nonrec mode =
  | Fullscreen(fullscreenMode)
  | Windowed(windowedMode);
type selection = (selectionStart, selectionEnd)
and selectionStart = int
and selectionEnd = int;
type nonrec currentFileName = option(string);
type uploadState =
  | Uploading
  | ReadyToUpload(uploadError)
and uploadError = option(string);
type nonrec state = {
  id: string,
  mode,
  selection,
  uploadState,
  currentFileName,
};
type nonrec action =
  | ClickPreview
  | ClickSplit
  | ClickFullscreen
  | SetSelection(selection)
  | BumpSelection(int)
  | PressEscapeKey
  | SetUploadError(uploadError)
  | SetUploading
  | FinishUploading
  | ClearFile
  | SelectFile(currentFileName);
let reducer = (state, action) =>
  switch (action) {
  | ClickPreview =>
    let mode =
      switch (state.mode) {
      | Windowed(`Preview) => Windowed(`Editor)
      | Windowed(`Editor) => Windowed(`Preview)
      | Fullscreen(`Editor)
      | Fullscreen(`Split) => Fullscreen(`Preview)
      | Fullscreen(`Preview) => Fullscreen(`Editor)
      };
    {...state, mode};
  | ClickSplit =>
    let mode =
      switch (state.mode) {
      | Windowed(_) => Fullscreen(`Split)
      | Fullscreen(`Editor)
      | Fullscreen(`Preview) => Fullscreen(`Split)
      | Fullscreen(`Split) => Fullscreen(`Editor)
      };
    {...state, mode};
  | ClickFullscreen =>
    let mode =
      switch (state.mode) {
      | Windowed(`Editor) => Fullscreen(`Editor)
      | Windowed(`Preview) => Fullscreen(`Preview)
      | Fullscreen(`Editor) => Windowed(`Editor)
      | Fullscreen(`Preview) => Windowed(`Preview)
      | Fullscreen(`Split) => Windowed(`Editor)
      };
    {...state, mode};
  | SetSelection(selection) => {...state, selection}
  | BumpSelection(offset) =>
    let (selectionStart, selectionEnd) = state.selection;
    {...state, selection: (selectionStart + offset, selectionEnd + offset)};
  | PressEscapeKey =>
    let mode =
      switch (state.mode) {
      | Fullscreen(`Editor) => Windowed(`Editor)
      | Windowed(`Preview)
      | Fullscreen(`Preview) => Windowed(`Preview)
      | Windowed(`Editor)
      | Fullscreen(`Split) => Windowed(`Editor)
      };
    {...state, mode};
  | SetUploadError(error) => {...state, uploadState: ReadyToUpload(error)}
  | SetUploading => {...state, uploadState: Uploading}
  | FinishUploading => {...state, uploadState: ReadyToUpload(None)}
  | SelectFile(currentFileName) => {...state, currentFileName}
  | ClearFile => {...state, currentFileName: None}
  };
let computeInitialState = ((value, textareaId, mode)) => {
  let id =
    switch (textareaId) {
    | Some(id) => id
    | None => DateTime.randomId()
    };
  let length = value |> String.length;
  {
    id,
    mode,
    selection: (length, length),
    uploadState: ReadyToUpload(None),
    currentFileName: None,
  };
};
let containerClasses = mode =>
  switch (mode) {
  | Windowed(_) => "relative bg-white overscroll-contain"
  | Fullscreen(_) => "bg-white fixed z-50 top-0 left-0 h-screen w-screen flex flex-col overscroll-contain"
  };
let modeIcon = (desiredMode, currentMode) => {
  let icon =
    switch (desiredMode, currentMode) {
    | (
        `Preview,
        Windowed(`Editor) | Fullscreen(`Editor) | Fullscreen(`Split),
      ) => "fas fa-eye"
    | (`Preview, Windowed(`Preview) | Fullscreen(`Preview)) => "fas fa-pen-nib"
    | (`Split, Windowed(_) | Fullscreen(`Editor) | Fullscreen(`Preview)) => "fas fa-columns"
    | (`Split, Fullscreen(`Split)) => "far fa-window-maximize"
    | (`Fullscreen, Windowed(_)) => "fas fa-expand"
    | (`Fullscreen, Fullscreen(_)) => "fas fa-compress"
    };
  <FaIcon classes={"fa-fw " ++ icon} />;
};
let modeLabel = (desiredMode, currentMode) =>
  switch (desiredMode, currentMode) {
  | (
      `Preview,
      Windowed(`Editor) | Fullscreen(`Editor) | Fullscreen(`Split),
    ) =>
    t("mode_label_preview")
  | (`Preview, Windowed(`Preview) | Fullscreen(`Preview)) =>
    t("mode_label_preview_exit")
  | (`Split, Windowed(_) | Fullscreen(`Editor) | Fullscreen(`Preview)) =>
    t("mode_label_split")
  | (`Split, Fullscreen(`Split)) => t("mode_label_split_exit")
  | (`Fullscreen, Windowed(_)) => t("mode_label_fullscreen")
  | (`Fullscreen, Fullscreen(_)) => t("mode_label_fullscreen_exit")
  };
let onClickFullscreen = (state, send, _event) => {
  switch (state.mode) {
  | Windowed(_) => TextareaAutosize.destroy(state.id)
  | Fullscreen(_) => ()
  };
  send(ClickFullscreen);
};
let onClickPreview = (state, send, _event) => {
  switch (state.mode) {
  | Windowed(`Editor) => TextareaAutosize.destroy(state.id)
  | Windowed(`Preview)
  | Fullscreen(_) => ()
  };
  send(ClickPreview);
};
let onClickSplit = (state, send, _event) => {
  switch (state.mode) {
  | Windowed(_) => TextareaAutosize.destroy(state.id)
  | Fullscreen(_) => ()
  };
  send(ClickSplit);
};
let insertAt = (textToInsert, position, sourceText) => {
  let head = sourceText->(String.sub(0, position));
  let tail =
    sourceText->(
                  String.sub(
                    position,
                    (sourceText |> String.length) - position,
                  )
                );
  head ++ textToInsert ++ tail;
};
let wrapWith = (wrapper, selectionStart, selectionEnd, sourceText) => {
  let head = sourceText->(String.sub(0, selectionStart));
  let selection =
    sourceText->(String.sub(selectionStart, selectionEnd - selectionStart));
  let tail =
    sourceText->(
                  String.sub(
                    selectionEnd,
                    (sourceText |> String.length) - selectionEnd,
                  )
                );
  head ++ wrapper ++ selection ++ wrapper ++ tail;
};
/**
  * After changing the Markdown using any of the controls or key commands, the
  * textarea element will need to be manually "synced" in two ways:
  *
  * 1. The autosize update function needs to be called to let it know that we
  *    have changed the value of the textare from the outside.
  * 2. The cursor position will have jumped to the end of the text-area because
  *    of the manual change of value of the controlled component; we'll need to
  *    manually set the cursor position after the component has had a change to
  *    re-render.
  *
  * This function is making an assumption that re-render can happen in 25ms.
  * The need for these manual adjustments can be visibly seen by increasing the
  * renderDelay to something like 1000ms.
 **/
let updateTextareaAfterDelay = (state, (startPosition, endPosition)) => {
  let renderDelay = 25;
  switch (state.mode) {
  | Windowed(_) =>
    Js.Global.setTimeout(
      ~f=() => TextareaAutosize.update(state.id),
      renderDelay,
    )
    |> ignore
  | Fullscreen(_) => ()
  };
  Webapi.Dom.(
    switch (document |> Document.getElementById(state.id)) {
    | Some(element) =>
      Js.Global.setTimeout(
        ~f=
          () => {
            element
            |> DomUtils.Element.unsafeToHtmlInputElement
            |> HtmlInputElement.setSelectionRange(startPosition, endPosition);
            Webapi.Dom.Document.getElementById(state.id, Webapi.Dom.document)
            ->(Belt.Option.flatMap(Webapi.Dom.Element.asHtmlElement))
            ->(Belt.Option.mapWithDefault((), Webapi.Dom.HtmlElement.focus));
          },
        renderDelay,
      )
      |> ignore
    | None => ()
    }
  );
};
let finalizeChange = (~newValue, ~state, ~send, ~onChange, ~offsetChange) => {
  let (selectionStart, selectionEnd) = state.selection;
  send(
    switch (offsetChange) {
    | `BumpSelection(offset) => BumpSelection(offset)
    | `SetSelection(selection) => SetSelection(selection)
    },
  );
  let (finalSelectionStart, finalSelectionEnd) =
    switch (offsetChange) {
    | `BumpSelection(offset) => (
        selectionStart + offset,
        selectionEnd + offset,
      )
    | `SetSelection(start, selectionEnd) => (start, selectionEnd)
    };
  onChange(newValue);
  updateTextareaAfterDelay(state, (finalSelectionStart, finalSelectionEnd));
};
type nonrec phraseModifer =
  | Bold
  | Italic
  | Strikethrough;
let insertAndWrapper = phraseModifer =>
  switch (phraseModifer) {
  | Bold => (({js|**|js} ++ t("bold_insert")) ++ {js|**|js}, "**")
  | Italic => (({js|*|js} ++ t("italic_insert")) ++ {js|*|js}, "*")
  | Strikethrough => (
      ({js|~~|js} ++ t("strikethrough_insert")) ++ {js|~~|js},
      "~~",
    )
  };
let modifyPhrase = (oldValue, state, send, onChange, phraseModifer) => {
  let (selectionStart, selectionEnd) = state.selection;
  let (insert, wrapper) = phraseModifer |> insertAndWrapper;
  let newValue =
    if (selectionStart == selectionEnd) {
      oldValue |> insertAt(insert, selectionStart);
    } else {
      oldValue |> wrapWith(wrapper, selectionStart, selectionEnd);
    };
  finalizeChange(
    ~newValue,
    ~state,
    ~send,
    ~onChange,
    ~offsetChange=
      if (selectionStart === selectionEnd) {
        `SetSelection((
          selectionStart + String.length(wrapper),
          selectionStart + String.length(insert) - String.length(wrapper),
        ));
      } else {
        `BumpSelection(String.length(wrapper));
      },
  );
};
let controlsContainerClasses = mode =>
  "border bg-gray-50 text-sm px-2 flex justify-between items-end "
  ++ (
    switch (mode) {
    | Windowed(_) => "rounded-t border-gray-300"
    | Fullscreen(_) => "border-gray-300 "
    }
  );
let controls = (disabled, value, state, send, onChange) => {
  let buttonClasses = "px-2 py-1 hover:bg-gray-300 hover:text-primary-500 focus:outline-none focus:bg-gray-300 focus:text-primary-500 ";
  switch (state) {
  | {mode, _} =>
    let curriedModifyPhrase = modifyPhrase(value, state, send, onChange);
    let valueReference = React.useRef(value);
    valueReference.current = value;
    let (currentSelectionStart, _) = state.selection;
    let selectionStart = React.useRef(currentSelectionStart);
    selectionStart.current = currentSelectionStart;
    let handleEmojiChange = (e: EmojiPicker.emojiEvent) =>
      onChange(
        valueReference.current |> insertAt(e.native, selectionStart.current),
      );
    <div className={controlsContainerClasses(state.mode)}>
      {switch (mode) {
       | Windowed(`Preview)
       | Fullscreen(`Preview) => <div />
       | Windowed(`Editor)
       | Fullscreen(`Editor | `Split) =>
         <div
           role="toolbar"
           className="bg-white border border-gray-300 rounded-t border-b-0">
           <button
             disabled
             ariaLabel={t("control_label_bold")}
             title={t("control_label_bold")}
             className=buttonClasses
             onClick={_ => curriedModifyPhrase(Bold)}>
             <i className="fas fa-bold fa-fw" />
           </button>
           <button
             disabled
             ariaLabel={t("control_label_italic")}
             title={t("control_label_italic")}
             className={buttonClasses ++ "border-l border-gray-300"}
             onClick={_ => curriedModifyPhrase(Italic)}>
             <i className="fas fa-italic fa-fw" />
           </button>
           <button
             disabled
             ariaLabel={t("control_label_strikethrough")}
             title={t("control_label_strikethrough")}
             className={buttonClasses ++ "border-l border-gray-300"}
             onClick={_ => curriedModifyPhrase(Strikethrough)}>
             <i className="fas fa-strikethrough fa-fw" />
           </button>
           <EmojiPicker
             onChange=handleEmojiChange
             className={
               buttonClasses ++ "border-l border-gray-400 hidden md:block"
             }
             title={t("emoji_picker")}
           />
         </div>
       }}
      <div className="py-1">
        <button
          ariaLabel={modeLabel(`Preview, mode)}
          title={modeLabel(`Preview, mode)}
          disabled
          className={"rounded " ++ buttonClasses}
          onClick={onClickPreview(state, send)}>
          {modeIcon(`Preview, mode)}
        </button>
        <button
          ariaLabel={modeLabel(`Split, mode)}
          title={modeLabel(`Split, mode)}
          disabled
          className={buttonClasses ++ "rounded ml-1 hidden md:inline"}
          onClick={onClickSplit(state, send)}>
          {modeIcon(`Split, mode)}
        </button>
        <button
          ariaLabel={modeLabel(`Fullscreen, mode)}
          title={modeLabel(`Fullscreen, mode)}
          disabled
          className={buttonClasses ++ "rounded  ml-1 hidden md:inline"}
          onClick={onClickFullscreen(state, send)}>
          {modeIcon(`Fullscreen, mode)}
          {switch (mode) {
           | Fullscreen(_) =>
             <span ariaHidden=true className="ml-2 text-xs font-semibold">
               {t("exit_full_screen_label")->str}
             </span>
           | Windowed(_) => React.null
           }}
        </button>
      </div>
    </div>;
  };
};
let modeClasses = mode =>
  switch (mode) {
  | Windowed(_) => ""
  | Fullscreen(_) => "flex grow"
  };
let editorContainerClasses = mode =>
  "border-r border-gray-300 "
  ++ (
    switch (mode) {
    | Windowed(`Editor) => "border-l"
    | Windowed(`Preview) => "hidden"
    | Fullscreen(`Editor) => "w-full"
    | Fullscreen(`Preview) => "hidden"
    | Fullscreen(`Split) => "w-1/2"
    }
  );
let previewType = mode =>
  switch (mode) {
  | Windowed(`Editor)
  | Fullscreen(`Editor) => raise(InvalidModeForPreview)
  | Windowed(`Preview) => `WindowedPreview
  | Fullscreen(`Split) => `FullscreenSplit
  | Fullscreen(`Preview) => `FullscreenPreview
  };
let previewContainerClasses = mode =>
  "border-gray-300 bg-gray-50 "
  ++ (
    switch (mode |> previewType) {
    | `WindowedPreview => "markdown-editor__windowed-preview-container border-l border-r border-b rounded-b px-2 md:px-3"
    | `FullscreenPreview => "w-screen mx-auto"
    | `FullscreenSplit => "w-1/2 relative"
    }
  );
let previewClasses = mode =>
  switch (mode) {
  | Fullscreen(`Split | `Preview) => "markdown-editor__fullscreen-preview-wrapper absolute max-h-full overflow-auto w-full px-4 pb-8"
  | Fullscreen(`Editor)
  | Windowed(_) => ""
  };
let focusOnEditor = id =>
  Webapi.Dom.(
    Document.getElementById(id, document)
    ->(Belt.Option.flatMap(HtmlElement.ofElement))
    ->(Belt.Option.mapWithDefault((), element => element->HtmlElement.focus))
  );
let handleUploadFileResponse = (oldValue, state, send, onChange, json) => {
  let errors = json |> Json.Decode.(field("errors", array(string)));
  if (errors == [||]) {
    let markdownEmbedCode =
      json |> Json.Decode.(field("markdownEmbedCode", string));
    let insert = "\n" ++ markdownEmbedCode ++ "\n";
    let (_, selectionEnd) = state.selection;
    let newValue = oldValue |> insertAt(insert, selectionEnd);
    finalizeChange(
      ~newValue,
      ~state,
      ~send,
      ~onChange,
      ~offsetChange=
        `BumpSelection(String.(length(newValue) - length(oldValue))),
    );
    send(FinishUploading);
  } else {
    send(
      SetUploadError(
        Some(
          (t("error_prefix") ++ " ") ++ (errors |> Js.Array.join(~sep=", ")),
        ),
      ),
    );
  };
};
let submitForm = (formId, oldValue, state, send, onChange) =>
  Webapi.Dom.(
    Document.getElementById(formId, document)
    ->(
        Belt.Option.mapWithDefault(
          (),
          element => {
            let formData = DomUtils.FormData.create(element);
            Api.sendFormData(
              "/markdown_attachments/",
              formData,
              handleUploadFileResponse(oldValue, state, send, onChange),
              () =>
              send(SetUploadError(Some(t("error_unexpected"))))
            );
          },
        )
      )
  );
let attachFile = (fileFormId, oldValue, state, send, onChange, event) =>
  switch (React.Event.Form.target(event)##files) {
  | [||] => ()
  | files =>
    let file = files[0];
    let maxFileSize = maxUploadFileSize;
    send(SelectFile(React.Event.Form.target(event)##value));
    let error =
      if (file##size > maxFileSize) {
        Some(t("error_maximum_file_size"));
      } else {
        None;
      };
    switch (error) {
    | Some(_) => send(SetUploadError(error))
    | None =>
      send(SetUploading);
      submitForm(fileFormId, oldValue, state, send, onChange);
      send(ClearFile);
    };
  };
let footerContainerClasses = mode =>
  "markdown-editor__footer-container border bg-gray-50 flex justify-end items-center "
  ++ (
    switch (mode) {
    | Windowed(_) => "rounded-b border-gray-300"
    | Fullscreen(_) => "border-gray-300"
    }
  );
let footer = (disabled, fileUpload, oldValue, state, send, onChange) =>
  switch (state) {
  | {id, _} =>
    let fileFormId = id ++ "-file-form";
    let fileInputId = id ++ "-file-input";
    switch (state.mode) {
    | Windowed(`Preview)
    | Fullscreen(`Preview) => React.null
    | Windowed(`Editor)
    | Fullscreen(`Editor | `Split) =>
      <div className={footerContainerClasses(state.mode)}>
        {<form
           className={
             {js|relative flex items-center flex-wrap flex-1 text-sm font-semibold |js}
             ++ (
               if (disabled) {""} else {
                 "hover:bg-gray-300 hover:text-primary-500 focus-within:outline-none focus-within:bg-gray-300 focus-within:text-primary-500"
               }
             )
           }
           id=fileFormId>
           <input
             name="authenticity_token"
             type_="hidden"
             value={AuthenticityToken.fromHead()}
           />
           <input
             className="absolute w-0 h-0 focus:outline-none"
             type_="file"
             name="markdown_attachment[file]"
             id=fileInputId
             multiple=false
             disabled
             value={
               switch (state.currentFileName) {
               | None => ""
               | Some(file) => file
               }
             }
             onChange={attachFile(
               fileFormId,
               oldValue,
               state,
               send,
               onChange,
             )}
           />
           {switch (state.uploadState) {
            | ReadyToUpload(error) =>
              <label
                className={
                  {js|text-xs px-3 py-2 flex grow |js}
                  ++ (
                    if (disabled) {"cursor-not-allowed"} else {
                      "cursor-pointer"
                    }
                  )
                }
                htmlFor=fileInputId>
                {switch (error) {
                 | Some(error) =>
                   <span className="text-red-500">
                     <i className="fas fa-exclamation-triangle mr-2" />
                     {error |> str}
                   </span>
                 | None =>
                   <span>
                     <i className="far fa-file-image mr-2" />
                     {t("attach_file_label")->str}
                   </span>
                 }}
              </label>
            | Uploading =>
              <span className="text-xs px-3 py-2 grow cursor-wait">
                <i className="fas fa-spinner fa-pulse mr-2" />
                {t("file_upload_wait")->str}
              </span>
            }}
         </form>
         ->(ReactUtils.nullUnless(fileUpload))}
        <a
          ariaLabel={t("help_aria_label")}
          href="/help/markdown_editor"
          target="_blank"
          className="flex items-center px-3 py-2 hover:bg-gray-300 hover:text-red-500 focus:outline-none focus:bg-gray-300 focus:text-red-500 cursor-pointer">
          <i className="fab fa-markdown text-sm" />
          <span className="text-xs ml-1 font-semibold hidden sm:inline">
            {t("help_label")->str}
          </span>
        </a>
      </div>
    };
  };
let textareaClasses = (mode, dynamicHeight) => {
  let editorClasses =
    if (dynamicHeight) {"w-full outline-none font-mono "} else {
      "markdown-editor__textarea w-full outline-none font-mono "
    };
  (editorClasses ++ "align-top focus:ring-1 focus:ring-focusColor-500 ")
  ++ (
    switch (mode) {
    | Windowed(_) => "p-3"
    | Fullscreen(_) => "markdown-editor__textarea--full-screen px-3 pt-4 pb-8 h-full resize-none"
    }
  );
};
let onChangeWrapper = (onChange, event) => {
  let value = React.Event.Form.target(event)##value;
  onChange(value);
};
let onSelect = (send, event) => {
  let htmlInputElement =
    React.Event.Selection.target(event)
    |> DomUtils.EventTarget.unsafeToHtmlInputElement;
  let selection =
    Webapi.Dom.(
      htmlInputElement |> HtmlInputElement.selectionStart,
      htmlInputElement |> HtmlInputElement.selectionEnd,
    );
  send(SetSelection(selection));
};
let handleEscapeKey = (send, event) =>
  switch (event |> Webapi.Dom.KeyboardEvent.key) {
  | "Escape" => send(PressEscapeKey)
  | _anyOtherKey => ()
  };
let handleKeyboardControls = (value, state, send, onChange, event) => {
  let ctrlKey = Webapi.Dom.KeyboardEvent.ctrlKey;
  let metaKey = Webapi.Dom.KeyboardEvent.metaKey;
  let curriedModifyPhrase = modifyPhrase(value, state, send, onChange);
  switch (event |> Webapi.Dom.KeyboardEvent.key) {
  | "b" when event |> ctrlKey || event |> metaKey => curriedModifyPhrase(Bold)
  | "i" when event |> ctrlKey || event |> metaKey =>
    curriedModifyPhrase(Italic)
  | _anyOtherKey => ()
  };
};
module ScrollSync = {
  open Webapi.Dom;
  let scrollTargetToSource = (~source, ~target, _event) => {
    let sourceScrollTop = source |> Element.scrollTop;
    let sourceOffsetHeight =
      source |> Element.unsafeAsHtmlElement |> HtmlElement.offsetHeight;
    let sourceScrollHeight = source |> Element.scrollHeight;
    let scrollFraction =
      sourceScrollTop
      /. (sourceScrollHeight - sourceOffsetHeight |> float_of_int);
    let maxTargetScrollTop =
      (target |> Element.scrollHeight)
      - (target |> Element.unsafeAsHtmlElement |> HtmlElement.offsetHeight)
      |> float_of_int;
    target->(Element.setScrollTop(scrollFraction *. maxTargetScrollTop));
  };
};
[@react.component]
let make =
    (
      ~value,
      ~onChange,
      ~profile,
      ~textareaId=?,
      ~maxLength=1000,
      ~defaultMode=Windowed(`Editor),
      ~placeholder=?,
      ~tabIndex=?,
      ~fileUpload=true,
      ~disabled=false,
      ~dynamicHeight=false,
    ) => {
  let (state, send) =
    React.useReducerWithMapState(
      reducer,
      (value, textareaId, defaultMode),
      computeInitialState,
    );
  React.useEffect1(
    () => {
      switch (state.mode) {
      | Windowed(`Editor) => TextareaAutosize.create(state.id)
      | Windowed(`Preview)
      | Fullscreen(_) => ()
      };
      Some(() => TextareaAutosize.destroy(state.id));
    },
    [|state.mode|],
  );
  React.useEffect0(() => {
    let curriedHandler = handleEscapeKey(send);
    let documentEventTarget = Webapi.Dom.(document |> Document.asEventTarget);
    documentEventTarget
    |> Webapi.Dom.EventTarget.addKeyDownEventListener(curriedHandler);
    Some(
      () =>
        documentEventTarget
        |> Webapi.Dom.EventTarget.removeKeyDownEventListener(curriedHandler),
    );
  });
  React.useEffect(() => {
    let curriedHandler = handleKeyboardControls(value, state, send, onChange);
    let textareaEventTarget =
      Webapi.Dom.(
        Document.getElementById(state.id, document)
        ->(Belt.Option.map(Element.asEventTarget))
      );
    textareaEventTarget->(
                           Belt.Option.mapWithDefault(
                             (),
                             Webapi.Dom.EventTarget.addKeyDownEventListener(
                               curriedHandler,
                             ),
                           )
                         );
    Some(
      () =>
        textareaEventTarget->(
                               Belt.Option.mapWithDefault(
                                 (),
                                 Webapi.Dom.EventTarget.removeKeyDownEventListener(
                                   curriedHandler,
                                 ),
                               )
                             ),
    );
  });
  React.useEffect1(
    () => {
      let textarea =
        Webapi.Dom.(document |> Document.getElementById(state.id));
      let preview =
        Webapi.Dom.(
          document |> Document.getElementById(state.id ++ "-preview")
        );
      switch (textarea, preview) {
      | (Some(textarea), Some(preview)) =>
        let scrollCallback =
          ScrollSync.scrollTargetToSource(~source=textarea, ~target=preview);
        switch (state.mode) {
        | Fullscreen(`Split) =>
          textarea
          |> Webapi.Dom.Element.addEventListener("scroll", scrollCallback);
          Some(
            () =>
              textarea
              |> Webapi.Dom.Element.removeEventListener(
                   "scroll",
                   scrollCallback,
                 ),
          );
        | _anyOtherMode =>
          textarea
          |> Webapi.Dom.Element.removeEventListener("scroll", scrollCallback);
          None;
        };
      | (_, _) => None
      };
    },
    [|state.mode|],
  );
  <div className={containerClasses(state.mode)}>
    {controls(disabled, value, state, send, onChange)}
    <div className={modeClasses(state.mode)}>
      <div className={editorContainerClasses(state.mode)}>
        <DisablingCover
          containerClasses="h-full"
          disabled={state.uploadState == Uploading}
          message="Uploading...">
          <textarea
            ?tabIndex
            ?placeholder
            ariaLabel="Markdown editor"
            rows=4
            maxLength
            onSelect={onSelect(send)}
            onChange={onChangeWrapper(onChange)}
            id={state.id}
            value
            className={textareaClasses(state.mode, dynamicHeight)}
            disabled
          />
        </DisablingCover>
      </div>
      {switch (state.mode) {
       | Windowed(`Editor)
       | Fullscreen(`Editor) => React.null
       | Windowed(`Preview)
       | Fullscreen(`Preview)
       | Fullscreen(`Split) =>
         <div className={previewContainerClasses(state.mode)}>
           <div
             id={state.id ++ "-preview"}
             className={previewClasses(state.mode)}>
             <MarkdownBlock
               markdown=value
               profile
               className="markdown-editor__fullscreen-preview-editor"
             />
           </div>
         </div>
       }}
    </div>
    {footer(disabled, fileUpload, value, state, send, onChange)}
  </div>;
};
