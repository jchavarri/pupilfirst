[%%raw {js|import "./CommunitiesShow__Root.css"|js}];
open CommunitiesShow__Types;
let str = React.string;
let t = I18n.t(~scope="components.CommunitiesShow__Root");
type nonrec solution = [ | `Solved | `Unsolved | `Unselected];
type nonrec sortCriterion = [ | `CreatedAt | `LastActivityAt | `Views];
type nonrec sortDirection = [ | `Ascending | `Descending];
type nonrec loading =
  | NotLoading
  | Reloading
  | LoadingMore;
type nonrec search =
  | SearchContent(string)
  | SearchTitle(string);
let getSearch = search =>
  switch (search) {
  | SearchContent(search) => search
  | SearchTitle(search) => search
  };
let searchBy = search =>
  switch (search) {
  | SearchContent(_) => "content"
  | SearchTitle(_) => "title"
  };
type nonrec filter = {
  topicCategory: option(TopicCategory.t),
  solution,
  search: option(search),
  target: option(Target.t),
  sortCriterion,
  sortDirection,
};
type nonrec state = {
  loading,
  topics: Topics.t,
  filterString: string,
  totalTopicsCount: int,
};
type nonrec action =
  | UpdateFilterString(string)
  | LoadTopics(option(string), bool, array(Topic.t), int)
  | BeginLoadingMore
  | BeginReloading;
let reducer = (state, action) =>
  switch (action) {
  | UpdateFilterString(filterString) => {...state, filterString}
  | LoadTopics(endCursor, hasNextPage, newTopics, totalTopicsCount) =>
    let updatedTopics =
      switch (state.loading) {
      | LoadingMore =>
        newTopics |> Array.append(state.topics |> Topics.toArray)
      | Reloading => newTopics
      | NotLoading => newTopics
      };
    {
      ...state,
      topics:
        switch (hasNextPage, endCursor) {
        | (_, None)
        | (false, Some(_)) => FullyLoaded(updatedTopics)
        | (true, Some(cursor)) => PartiallyLoaded(updatedTopics, cursor)
        },
      loading: NotLoading,
      totalTopicsCount,
    };
  | BeginLoadingMore => {...state, loading: LoadingMore}
  | BeginReloading => {...state, loading: Reloading}
  };
module TopicsQuery = [%graphql
  {js|
    query TopicsFromCommunitiesShowRootQuery($communityId: ID!, $topicCategoryId: ID,$targetId: ID, $resolution: TopicResolutionFilter!, $searchFilter: CommunitySearchFilter, $after: String, $sortCriterion: TopicSortCriterion!, $sortDirection: SortDirection!) {
      topics(communityId: $communityId, topicCategoryId: $topicCategoryId,targetId: $targetId, search: $searchFilter, resolution: $resolution, sortDirection: $sortDirection, sortCriterion: $sortCriterion, first: 10, after: $after) {
        nodes {
          id
          lastActivityAt
          createdAt
          creator {
            name
          }
          likesCount
          liveRepliesCount
          views
          title
          solved
          topicCategoryId
          participantsCount
          participants {
            id
            name
            avatarUrl
          }
        }
        pageInfo{
          endCursor,hasNextPage
        }
        totalCount
      }
    }
  |js}
];
let getTopics = (send, communityId, cursor, filter) => {
  let topicCategoryId =
    filter.topicCategory |> OptionUtils.map(TopicCategory.id);
  let targetId = filter.target |> OptionUtils.map(Target.id);
  let searchFilter =
    Belt.Option.map(filter.search, search =>
      switch (search) {
      | SearchContent(searchString) =>
        TopicsQuery.makeInputObjectCommunitySearchFilter(
          ~search=searchString,
          ~searchBy=`content,
          (),
        )
      | SearchTitle(searchString) =>
        TopicsQuery.makeInputObjectCommunitySearchFilter(
          ~search=searchString,
          ~searchBy=`title,
          (),
        )
      }
    );
  let variables =
    TopicsQuery.makeVariables(
      ~communityId,
      ~after=?cursor,
      ~topicCategoryId?,
      ~targetId?,
      ~searchFilter?,
      ~resolution=filter.solution,
      ~sortCriterion=filter.sortCriterion,
      ~sortDirection=filter.sortDirection,
      (),
    );
  TopicsQuery.make(variables)
  |> Js.Promise.then_(response => {
       let newTopics =
         response##topics##nodes
         |> Js.Array.map(~f=topicData => Topic.makeFromJS(topicData));
       send(
         LoadTopics(
           response##topics##pageInfo##endCursor,
           response##topics##pageInfo##hasNextPage,
           newTopics,
           response##topics##totalCount,
         ),
       );
       Js.Promise.resolve();
     })
  |> ignore;
};
let reloadTopics = (communityId, filter, send) => {
  send(BeginReloading);
  getTopics(send, communityId, None, filter);
};
let computeInitialState = () => {
  loading: NotLoading,
  topics: Unloaded,
  filterString: "",
  totalTopicsCount: 0,
};
let filterToQueryString = filter => {
  let sortCriterion =
    switch (filter.sortCriterion) {
    | `LastActivityAt => "LastActivityAt"
    | `Views => "Views"
    | `CreatedAt => "CreatedAt"
    };
  let sortDirection =
    switch (filter.sortDirection) {
    | `Descending => "Descending"
    | `Ascending => "Ascending"
    };
  let filterDict =
    Js.Dict.fromArray([|
      ("sortCriterion", sortCriterion),
      ("sortDirection", sortDirection),
    |]);
  Belt.Option.forEach(
    filter.search,
    search => {
      Js.Dict.set(filterDict, "search", getSearch(search));
      Js.Dict.set(filterDict, "searchBy", searchBy(search));
    },
  );
  Belt.Option.forEach(filter.topicCategory, tc =>
    Js.Dict.set(filterDict, "topicCategory", TopicCategory.id(tc))
  );
  switch (filter.solution) {
  | `Solved => Js.Dict.set(filterDict, "solution", "Solved")
  | `Unsolved => Js.Dict.set(filterDict, "solution", "Unsolved")
  | `Unselected => ()
  };
  Webapi.Url.(
    URLSearchParams.makeWithDict(filterDict)->URLSearchParams.toString
  );
};
let updateParams = filter =>
  ReasonReactRouter.push("?" ++ filterToQueryString(filter));
let topicsList = (topicCategories, topics) =>
  if (topics |> ArrayUtils.isEmpty) {
    <div
      className="flex flex-col mx-auto bg-white rounded-md border p-6 justify-center items-center">
      <FaIcon classes="fas fa-comments text-5xl text-gray-400" />
      <h4 className="mt-3 text-base md:text-lg text-center font-semibold">
        {t("empty_topics")->str}
      </h4>
    </div>;
  } else {
    topics
    |> Js.Array.map(~f=topic =>
         <a
           className="block"
           key={Topic.id(topic)}
           href={"/topics/" ++ Topic.id(topic)}
           ariaLabel={"Topic " ++ Topic.id(topic)}>
           <div
             className="flex items-center border border-transparent hover:bg-gray-50 hover:text-primary-500  hover:border-primary-400">
             <div className="flex-1">
               <div
                 className="cursor-pointer no-underline flex flex-col p-4 md:px-6 md:py-5">
                 <span className="block">
                   <h4
                     className="community-topic__title text-base font-semibold inline-block break-words leading-snug">
                     {Topic.title(topic) |> str}
                   </h4>
                   <span className="block text-xs text-gray-800 pt-1">
                     <span> {t("topic_posted_by_text")->str} </span>
                     <span className="font-semibold mr-2">
                       {(
                          switch (Topic.creatorName(topic)) {
                          | Some(name) => " " ++ name ++ " "

                          | None => "Unknown "
                          }
                        )
                        |> str}
                     </span>
                     <span className="hidden md:inline-block md:mr-2">
                       {"on "
                        ++ Topic.createdAt(topic)
                           ->(DateFns.formatPreset(~year=true, ()))
                        |> str}
                     </span>
                     <span
                       className="inline-block md:mt-0 md:px-2 md:border-l border-gray-300">
                       {switch (Topic.lastActivityAt(topic)) {
                        | Some(date) =>
                          <span>
                            <span className="hidden md:inline-block">
                              {t("topic_last_updated_text")->str}
                            </span>
                            {" "
                             ++ DateFns.formatDistanceToNowStrict(
                                  date,
                                  ~addSuffix=true,
                                  (),
                                )
                             |> str}
                          </span>

                        | None => React.null
                        }}
                     </span>
                   </span>
                 </span>
                 <span className="flex flex-row flex-wrap mt-2 items-center">
                   <span
                     className="flex text-center items-center mr-2 py-1 px-2 rounded bg-gray-50"
                     ariaLabel="Likes">
                     <i
                       className="far fa-thumbs-up text-sm text-gray-600 mr-1"
                     />
                     <p className="text-xs font-semibold">
                       <span className="ml-1 hidden md:inline">
                         {t(
                            ~count=Topic.likesCount(topic),
                            "topic_stats_likes",
                          )
                          ->str}
                       </span>
                     </p>
                   </span>
                   <span
                     className="flex justify-between text-center items-center mr-2 py-1 px-2 rounded bg-gray-50"
                     ariaLabel="Replies">
                     <i
                       className="far fa-comment-dots text-sm text-gray-600 mr-1"
                     />
                     <p className="text-xs font-semibold">
                       <span className="ml-1 hidden md:inline">
                         {t(
                            ~count=Topic.liveRepliesCount(topic),
                            "topic_stats_replies",
                          )
                          ->str}
                       </span>
                     </p>
                   </span>
                   <span
                     className="flex justify-between text-center items-center mr-2 py-1 px-2 rounded bg-gray-50"
                     ariaLabel="Views">
                     <i className="far fa-eye text-sm text-gray-600 mr-1" />
                     <p className="text-xs font-semibold">
                       <span className="ml-1 hidden md:inline">
                         {t(~count=Topic.views(topic), "topic_stats_views")
                          ->str}
                       </span>
                     </p>
                   </span>
                   {switch (Topic.topicCategoryId(topic)) {
                    | Some(id) =>
                      let topicCategory =
                        topicCategories
                        |> ArrayUtils.unsafeFind(
                             c => TopicCategory.id(c) == id,
                             t("unable_find_id") ++ id,
                           );
                      let (color, _) =
                        StringUtils.toColor(
                          TopicCategory.name(topicCategory),
                        );
                      let style =
                        ReactDOM.Style.make(~backgroundColor=color, ());
                      <span
                        className="flex items-center text-xs font-semibold py-1 mr-2">
                        <div className="w-3 h-3 rounded" style />
                        <span className="ml-1">
                          {TopicCategory.name(topicCategory)->str}
                        </span>
                      </span>;

                    | None => React.null
                    }}
                   {ReactUtils.nullUnless(
                      <span
                        ariaLabel="Solved status icon"
                        className="flex items-center justify-center w-5 h-5 bg-green-200 text-green-800 rounded-full">
                        <PfIcon className="if i-check-solid text-xs" />
                      </span>,
                      Topic.solved(topic),
                    )}
                 </span>
               </div>
             </div>
             <div className="md:w-1/5">
               <CommunitiesShow__Participants
                 title=React.null
                 className="flex shrink-0 items-center justify-end pr-4 md:pr-6"
                 participants={Topic.participants(topic)}
                 participantsCount={Topic.participantsCount(topic)}
               />
             </div>
           </div>
         </a>
       )
    |> React.array;
  };
let topicsLoadedData = (totalTopicsCount, loadedTopicsCount) =>
  <div
    className="inline-block mt-2 mx-auto bg-gray-50 text-gray-800 text-xs p-2 text-center rounded font-semibold">
    {(
       if (totalTopicsCount == loadedTopicsCount) {
         t(
           ~variables=[|("total_topics", string_of_int(totalTopicsCount))|],
           "topics_fully_loaded_text",
         );
       } else {
         t(
           ~variables=[|
             ("total_topics", string_of_int(totalTopicsCount)),
             ("loaded_topics_count", string_of_int(loadedTopicsCount)),
           |],
           "topics_partially_loaded_text",
         );
       }
     )
     |> str}
  </div>;
module Selectable = {
  type nonrec t =
    | TopicCategory(TopicCategory.t)
    | Solution(bool)
    | Search(search);
  let label = t =>
    switch (t) {
    | TopicCategory(_category) => Some("Category")
    | Search(SearchTitle(_)) => Some("Search by title")
    | Search(SearchContent(_)) => Some("Search by content")
    | Solution(_) => Some("Solution")
    };
  let value = t =>
    switch (t) {
    | TopicCategory(category) => TopicCategory.name(category)
    | Search(search) => getSearch(search)
    | Solution(solution) => if (solution) {"Solved"} else {"Unsolved"}
    };
  let searchString = t =>
    switch (t) {
    | TopicCategory(category) => "category " ++ TopicCategory.name(category)
    | Search(search) => getSearch(search)
    | Solution(_solution) => "solution solved unsolved"
    };
  let color = t =>
    switch (t) {
    | TopicCategory(_category) => "orange"
    | Search(_search) => "gray"
    | Solution(_solution) => "green"
    };
  let topicCategory = topicCategory => TopicCategory(topicCategory);
  let search = search => Search(search);
  let solution = on => Solution(on);
};
module Multiselect = MultiselectDropdown.Make(Selectable);
let unselected = (topicCategories, filter, state) => {
  let unselectedCategories =
    topicCategories
    |> Js.Array.filter(~f=category =>
         filter.topicCategory
         |> OptionUtils.mapWithDefault(
              selectedCategory =>
                category->TopicCategory.id
                != selectedCategory->TopicCategory.id,
              true,
            )
       )
    |> Array.map(Selectable.topicCategory);
  let trimmedFilterString = state.filterString |> String.trim;
  let search =
    if (trimmedFilterString == "") {
      [||];
    } else {
      [|
        Selectable.search(SearchContent(trimmedFilterString)),
        Selectable.search(SearchTitle(trimmedFilterString)),
      |];
    };
  let hasSolution =
    switch (filter.solution) {
    | `Solved => [|Selectable.solution(false)|]
    | `Unsolved => [|Selectable.solution(true)|]
    | `Unselected => [|
        Selectable.solution(true),
        Selectable.solution(false),
      |]
    };
  unselectedCategories
  |> Js.Array.concat(~other=search)
  |> Js.Array.concat(~other=hasSolution);
};
let selected = filter => {
  let selectedCategory =
    filter.topicCategory
    |> OptionUtils.mapWithDefault(
         selectedCategory => [|Selectable.topicCategory(selectedCategory)|],
         [||],
       );
  let selectedSearchString =
    filter.search
    |> OptionUtils.mapWithDefault(
         search => [|Selectable.search(search)|],
         [||],
       );
  let selectedSolutionFilter =
    switch (filter.solution) {
    | `Solved => [|Selectable.solution(true)|]
    | `Unsolved => [|Selectable.solution(false)|]
    | `Unselected => [||]
    };
  selectedCategory
  |> Js.Array.concat(~other=selectedSearchString)
  |> Js.Array.concat(~other=selectedSolutionFilter);
};
let onSelectFilter = (filter, send, selectable) => {
  switch (selectable) {
  | Selectable.TopicCategory(topicCategory) =>
    updateParams({...filter, topicCategory: Some(topicCategory)})
  | Search(search) => updateParams({...filter, search: Some(search)})
  | Solution(onOrOff) =>
    let solution =
      if (onOrOff) {
        `Solved;
      } else {
        `Unsolved;
      };
    updateParams({...filter, solution});
  };
  send(UpdateFilterString(""));
};
let onDeselectFilter = (filter, selectable) =>
  switch (selectable) {
  | Selectable.TopicCategory(_topicCategory) =>
    updateParams({...filter, topicCategory: None})
  | Search(_search) => updateParams({...filter, search: None})
  | Solution(_) => updateParams({...filter, solution: `Unselected})
  };
let filterPlaceholder = (filter, topicCategories) =>
  switch (filter.topicCategory, filter.search) {
  | (None, None) =>
    if (ArrayUtils.isEmpty(topicCategories)) {
      t("filter_input_placeholder_default");
    } else {
      t("filter_input_placeholder_categories");
    }
  | _ => ""
  };
let categoryDropdownSelected = topicCategory =>
  <div
    ariaLabel="Selected category filter"
    className="text-sm bg-white border border-gray-300 rounded py-1 md:py-2 px-3 focus:outline-none focus:bg-white focus:border-primary-300 cursor-pointer">
    {switch (topicCategory) {
     | Some(topicCategory) =>
       let (color, _) = TopicCategory.color(topicCategory);
       let style = ReactDOM.Style.make(~backgroundColor=color, ());
       <div className="inline-flex items-center">
         <div className="h-3 w-3 border" style />
         <span className="ml-2">
           {TopicCategory.name(topicCategory)->str}
         </span>
       </div>;
     | None => t("all_categories_button")->str
     }}
    <FaIcon classes="ml-4 fas fa-caret-down" />
  </div>;
let categoryDropdownContents = (availableTopicCategories, filter) => {
  let selectableTopicCategories =
    Belt.Option.mapWithDefault(
      filter.topicCategory, availableTopicCategories, topicCategory =>
      Js.Array.filter(
        ~f=
          availableTopicCategory =>
            TopicCategory.id(availableTopicCategory)
            != TopicCategory.id(topicCategory),
        availableTopicCategories,
      )
    );
  Js.Array.map(
    ~f=
      topicCategory => {
        let (color, _) = TopicCategory.color(topicCategory);
        let style = ReactDOM.Style.make(~backgroundColor=color, ());
        let categoryName = TopicCategory.name(topicCategory);
        <div
          ariaLabel={"Select category " ++ categoryName}
          className="pl-3 pr-4 py-2 font-normal flex items-center"
          onClick={_ =>
            updateParams({...filter, topicCategory: Some(topicCategory)})
          }>
          <div className="w-3 h-3 rounded" style />
          <span className="ml-1"> categoryName->str </span>
        </div>;
      },
    selectableTopicCategories,
  );
};
module Sortable = {
  type nonrec t = sortCriterion;
  let criterion = c =>
    switch (c) {
    | `CreatedAt => t("sort_criterion_posted_at")
    | `LastActivityAt => t("sort_criterion_last_activity")
    | `Views => t("sort_criterion_views")
    };
  let criterionType = _t => `Number;
};
module TopicsSorter = Sorter.Make(Sortable);
let topicsSorter = filter =>
  <div ariaLabel="Change topics sorting" className="shrink-0">
    <label className="block text-tiny font-semibold uppercase">
      {t("sort_criterion_input_label")->str}
    </label>
    <TopicsSorter
      criteria=[|`CreatedAt, `LastActivityAt, `Views|]
      selectedCriterion={filter.sortCriterion}
      direction={filter.sortDirection}
      onDirectionChange={sortDirection =>
        updateParams({...filter, sortDirection})
      }
      onCriterionChange={sortCriterion =>
        updateParams({...filter, sortCriterion})
      }
    />
  </div>;
let filterFromQueryParams = (search, target, topicCategories) => {
  let params = Webapi.Url.URLSearchParams.make(search);
  Webapi.Url.URLSearchParams.{
    search:
      get("search", params)
      ->(
          Belt.Option.map(searchString =>
            switch (get("searchBy", params)) {
            | None => SearchTitle(searchString)
            | Some("content") => SearchContent(searchString)
            | Some("title") => SearchTitle(searchString)
            | Some(_) => SearchTitle(searchString)
            }
          )
        ),
    topicCategory:
      get("topicCategory", params)
      ->(
          Belt.Option.flatMap(cat =>
            Js.Array.find(
              ~f=c => TopicCategory.id(c) == cat,
              topicCategories,
            )
          )
        ),
    target,
    solution:
      switch (get("solution", params)) {
      | Some(criterion) when criterion == "Solved" => `Solved
      | Some(criterion) when criterion == "Unsolved" => `Unsolved
      | _ => `Unselected
      },
    sortCriterion:
      switch (get("sortCriterion", params)) {
      | Some(criterion) when criterion == "LastActivityAt" => `LastActivityAt
      | Some(criterion) when criterion == "Views" => `Views
      | Some(criterion) when criterion == "CreatedAt" => `CreatedAt
      | _ => `CreatedAt
      },
    sortDirection:
      switch (get("sortDirection", params)) {
      | Some(direction) when direction == "Descending" => `Descending
      | Some(direction) when direction == "Ascending" => `Ascending
      | _ => `Descending
      },
  };
};
[@react.component]
let make = (~communityId, ~target, ~topicCategories) => {
  let (state, send) = React.useReducer(reducer, computeInitialState());
  let url = ReasonReactRouter.useUrl();
  let filter = filterFromQueryParams(url.search, target, topicCategories);
  React.useEffect1(
    () => {
      reloadTopics(communityId, filter, send);
      None;
    },
    [|url|],
  );
  <div className="flex-1 flex flex-col">
    {switch (target) {
     | Some(target) =>
       <div className="max-w-3xl w-full mt-5 mx-auto px-3 md:px-0">
         <div
           className="flex py-4 px-4 md:px-6 w-full bg-yellow-100 border border-dashed border-yellow-400 rounded justify-between items-center">
           <p className="w-3/5 md:w-4/5 font-semibold text-sm">
             {"Target: " ++ Target.title(target) |> str}
           </p>
           <a
             className="no-underline bg-yellow-100 border border-yellow-400 px-3 py-2 hover:bg-yellow-200 rounded-lg cursor-pointer text-xs font-semibold"
             href={"/communities/" ++ communityId}>
             {"Clear Filter" |> str}
           </a>
         </div>
       </div>
     | None => React.null
     }}
    <div className="mt-5 flex flex-col flex-1 ">
      <div className="w-full sticky top-0 z-30 bg-gray-50 py-2">
        <div className="max-w-3xl w-full mx-auto relative px-3 md:px-6">
          <div className="pb-3 flex justify-between">
            {ReactUtils.nullIf(
               <Dropdown
                 selected={categoryDropdownSelected(filter.topicCategory)}
                 contents={categoryDropdownContents(topicCategories, filter)}
                 className=""
               />,
               ArrayUtils.isEmpty(topicCategories),
             )}
            <div className="" />
          </div>
          <div className="flex w-full items-start flex-wrap">
            <div className="flex-1 pr-2">
              <label className="block text-tiny font-semibold uppercase pl-px">
                {t("filter_input_label")->str}
              </label>
              <Multiselect
                id="filter"
                unselected={unselected(topicCategories, filter, state)}
                selected={selected(filter)}
                onSelect={onSelectFilter(filter, send)}
                onDeselect={onDeselectFilter(filter)}
                value={state.filterString}
                onChange={filterString =>
                  send(UpdateFilterString(filterString))
                }
                placeholder={filterPlaceholder(filter, topicCategories)}
              />
            </div>
            {topicsSorter(filter)}
          </div>
        </div>
      </div>
      <div className="max-w-3xl w-full mx-auto relative px-3 md:px-6 pb-4">
        <div id="topics" className="community-topic__list-container my-4">
          {switch (state.topics) {
           | Unloaded =>
             SkeletonLoading.multiple(
               ~count=10,
               ~element=SkeletonLoading.card(),
             )
           | PartiallyLoaded(topics, cursor) =>
             <div>
               <div className="shadow bg-white rounded-lg divide-y">
                 {topicsList(topicCategories, topics)}
               </div>
               <div className="text-center">
                 {topicsLoadedData(
                    state.totalTopicsCount,
                    Array.length(topics),
                  )}
               </div>
               {switch (state.loading) {
                | LoadingMore =>
                  SkeletonLoading.multiple(
                    ~count=3,
                    ~element=SkeletonLoading.card(),
                  )

                | NotLoading =>
                  <button
                    className="btn btn-primary-ghost cursor-pointer w-full mt-4"
                    onClick={_ => {
                      send(BeginLoadingMore);
                      getTopics(send, communityId, Some(cursor), filter);
                    }}>
                    {t("button_load_more") |> str}
                  </button>

                | Reloading => React.null
                }}
             </div>
           | FullyLoaded(topics) =>
             <div>
               <div className="shadow bg-white rounded-lg divide-y">
                 {topicsList(topicCategories, topics)}
               </div>
               <div className="text-center">
                 {topicsLoadedData(
                    state.totalTopicsCount,
                    Array.length(topics),
                  )}
               </div>
             </div>
           }}
        </div>
      </div>
      {switch (state.topics) {
       | Unloaded => React.null
       | _ =>
         let loading =
           switch (state.loading) {
           | NotLoading => false
           | Reloading => true
           | LoadingMore => false
           };
         <LoadingSpinner loading />;
       }}
    </div>
  </div>;
};
