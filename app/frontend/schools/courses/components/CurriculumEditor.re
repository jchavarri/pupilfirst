let str = React.string;
let t = I18n.t(~scope="components.CurriculumEditor");
open CurriculumEditor__Types;
type nonrec editorAction =
  | Hidden
  | ShowTargetGroupEditor(option(TargetGroup.t))
  | ShowLevelEditor(option(Level.t));
type nonrec state = {
  selectedLevel: Level.t,
  editorAction,
  levels: array(Level.t),
  targetGroups: array(TargetGroup.t),
  targets: array(Target.t),
  showArchived: bool,
};
type nonrec action =
  | SelectLevel(Level.t)
  | UpdateEditorAction(editorAction)
  | UpdateLevels(Level.t)
  | UpdateTargetGroup(TargetGroup.t)
  | UpdateTargetGroups(array(TargetGroup.t))
  | UpdateTarget(Target.t)
  | UpdateTargets(array(Target.t))
  | ToggleShowArchived;
let reducer = (state, action) =>
  switch (action) {
  | SelectLevel(selectedLevel) => {...state, selectedLevel}
  | UpdateEditorAction(editorAction) => {...state, editorAction}
  | UpdateLevels(level) =>
    let newLevels = level |> Level.updateArray(state.levels);
    {...state, levels: newLevels, editorAction: Hidden, selectedLevel: level};
  | UpdateTargetGroup(targetGroup) =>
    let newtargetGroups =
      targetGroup |> TargetGroup.updateArray(state.targetGroups);
    {...state, targetGroups: newtargetGroups};
  | UpdateTargetGroups(targetGroups) => {...state, targetGroups}
  | UpdateTarget(target) =>
    let newtargets = target |> Target.updateArray(state.targets);
    {...state, targets: newtargets};
  | ToggleShowArchived => {...state, showArchived: !state.showArchived}
  | UpdateTargets(targets) => {...state, targets}
  };
let showArchivedButton = (targetGroupsInLevel, targets) => {
  let tgIds = targetGroupsInLevel |> Js.Array.map(~f=TargetGroup.id);
  let numberOfArchivedTargetGroupsInLevel =
    targetGroupsInLevel
    |> Js.Array.filter(~f=TargetGroup.archived)
    |> Js.Array.length;
  let numberOfArchivedTargetsInLevel =
    targets
    |> Js.Array.filter(~f=target =>
         tgIds |> Js.Array.includes(~value=Target.targetGroupId(target))
       )
    |> Js.Array.filter(~f=target => Target.visibility(target) == Archived)
    |> Js.Array.length;
  numberOfArchivedTargetGroupsInLevel > 0 || numberOfArchivedTargetsInLevel > 0;
};
let updateTargetSortIndex = (state, send, sortedTargets) => {
  let oldTargets =
    state.targets
    |> Js.Array.filter(~f=t => !Js.Array.includes(~value=t, sortedTargets));
  send(
    UpdateTargets(
      Js.Array.concat(
        oldTargets,
        ~other=Target.updateSortIndex(sortedTargets),
      ),
    ),
  );
};
let updateTargetGroupSortIndex = (state, send, sortedTargetGroups) => {
  let oldTargetGroups =
    state.targetGroups
    |> Js.Array.filter(~f=t =>
         !Js.Array.includes(~value=t, sortedTargetGroups)
       );
  send(
    UpdateTargetGroups(
      Js.Array.concat(
        TargetGroup.updateSortIndex(sortedTargetGroups),
        ~other=oldTargetGroups,
      ),
    ),
  );
};
let levelOfTarget = (targetId, targets, levels, targetGroups) => {
  let target =
    targets
    |> ArrayUtils.unsafeFind(
         target => Target.id(target) == targetId,
         "Unable to find target with ID:" ++ targetId ++ " in CurriculumEditor",
       );
  let targetGroup =
    targetGroups
    |> ArrayUtils.unsafeFind(
         tg => TargetGroup.id(tg) == Target.targetGroupId(target),
         "Unable to find target group with ID:"
         ++ Target.targetGroupId(target)
         ++ " in CurriculumEditor",
       );
  Level.unsafeFind(
    levels,
    "CurriculumEditor",
    TargetGroup.levelId(targetGroup),
  );
};
let computeIntialState = ((levels, targetGroups, targets, path)) => {
  let defaultLevel =
    DomUtils.getUrlParam(~key="level")
    ->(Belt.Option.flatMap(Belt.Int.fromString))
    ->(
        Belt.Option.flatMap(levelNumber =>
          Js.Array.find(
            ~f=level => Level.number(level) == levelNumber,
            levels,
          )
        )
      )
    ->(
        Belt.Option.getWithDefault(
          Js.Array.reduce(
            levels,
            ~f=
              (max, level) =>
                if (Level.number(level) > Level.number(max)) {
                  level;
                } else {
                  max;
                },
            ~init=Js.Array.unsafe_get(levels, 0),
          ),
        )
      );
  let selectedLevel =
    switch (path) {
    | ["school", "courses", _courseId, "curriculum"] => defaultLevel
    | ["school", "courses", _courseId, "targets", targetId, ..._] =>
      levelOfTarget(targetId, targets, levels, targetGroups)
    | _ => defaultLevel
    };
  {
    selectedLevel,
    editorAction: Hidden,
    targetGroups,
    levels,
    targets,
    showArchived: false,
  };
};
[@react.component]
let make =
    (
      ~course,
      ~evaluationCriteria,
      ~levels,
      ~targetGroups,
      ~targets,
      ~hasVimeoAccessToken,
      ~vimeoPlan,
      ~markdownCurriculumEditorMaxLength,
    ) => {
  let path = ReasonReactRouter.useUrl().path;
  let (state, send) =
    React.useReducerWithMapState(
      reducer,
      (levels, targetGroups, targets, path),
      computeIntialState,
    );
  let hideEditorActionCB = () => send(UpdateEditorAction(Hidden));
  let currentLevel = state.selectedLevel;
  let currentLevelId = Level.id(currentLevel);
  let updateLevelsCB = level => send(UpdateLevels(level));
  let targetGroupsInLevel =
    state.targetGroups
    |> Js.Array.filter(~f=targetGroup =>
         TargetGroup.levelId(targetGroup) == currentLevelId
       )
    |> TargetGroup.sort;
  let targetGroupsToDisplay =
    if (state.showArchived) {
      targetGroupsInLevel;
    } else {
      targetGroupsInLevel
      |> Js.Array.filter(~f=tg => !TargetGroup.archived(tg));
    };
  let showTargetGroupEditorCB = targetGroup =>
    send(UpdateEditorAction(ShowTargetGroupEditor(targetGroup)));
  let updateTargetCB = target => {
    let targetGroup =
      state.targetGroups
      |> ArrayUtils.unsafeFind(
           tg => TargetGroup.id(tg) == Target.targetGroupId(target),
           "Unable to find target group with ID:"
           ++ Target.targetGroupId(target),
         );
    let updatedTargetGroup =
      switch (target |> Target.visibility) {
      | Archived => targetGroup
      | Draft
      | Live => targetGroup |> TargetGroup.unarchive
      };
    send(UpdateTarget(target));
    send(UpdateTargetGroup(updatedTargetGroup));
  };
  let updateTargetGroupsCB = targetGroup => {
    if (targetGroup |> TargetGroup.archived) {
      let targetIdsInTargerGroup =
        state.targets
        |> Target.targetIdsInTargetGroup(targetGroup |> TargetGroup.id);
      let newTargets =
        state.targets
        |> Js.Array.map(~f=target =>
             if (targetIdsInTargerGroup
                 |> Js.Array.includes(~value=Target.id(target))) {
               Target.archive(target);
             } else {
               target;
             }
           );
      send(UpdateTargets(newTargets));
    } else {
      ();
    };
    send(UpdateTargetGroup(targetGroup));
    send(UpdateEditorAction(Hidden));
  };
  <div className="flex-1 flex flex-col">
    <div className="bg-white p-4 md:hidden shadow border-b">
      <button
        className="sa-toggle__menu-btn sa-toggle__menu-btn--arrow hover:bg-gray-50 focus:outline-none">
        <span className="sa-toggle__menu-btn-box">
          <span className="sa-toggle__menu-btn-inner" />
        </span>
      </button>
    </div>
    <CurriculumEditor__TargetDrawer
      hasVimeoAccessToken
      markdownCurriculumEditorMaxLength
      targets={state.targets}
      levels={state.levels}
      targetGroups={state.targetGroups}
      evaluationCriteria
      course
      updateTargetCB
      vimeoPlan
    />
    {switch (state.editorAction) {
     | Hidden => React.null
     | ShowTargetGroupEditor(targetGroup) =>
       <CurriculumEditor__TargetGroupEditor
         targetGroup
         levels={state.levels}
         currentLevelId
         updateTargetGroupsCB
         hideEditorActionCB
       />
     | ShowLevelEditor(level) =>
       <CurriculumEditor__LevelEditor
         levels={state.levels}
         level
         course
         hideEditorActionCB
         updateLevelsCB
       />
     }}
    <div className="px-6 pb-12 flex-1 bg-gray-50 relative">
      <div
        className="w-full py-4 relative md:sticky top-0 z-20 bg-gray-50 border-b">
        <div className="max-w-3xl flex items-center justify-between mx-auto">
          <div className="flex">
            <div className="inline-block relative w-auto md:w-64">
              <select
                onChange={event => {
                  let level_name = React.Event.Form.target(event)##value;
                  send(
                    SelectLevel(Level.selectLevel(state.levels, level_name)),
                  );
                }}
                value={currentLevel |> Level.name}
                ariaLabel="Select level"
                className="block appearance-none w-full bg-white border text-sm border-gray-300 rounded-l hover:border-gray-500 px-4 py-3 pr-8 rounded-r-none leading-tight focus:outline-none focus:ring-2 focus:ring-inset focus:ring-focusColor-500">
                {state.levels
                 |> Level.sort
                 |> Array.map(level =>
                      <option
                        key={Level.id(level)} value={level |> Level.name}>
                        {LevelLabel.format(
                           ~name=level |> Level.name,
                           level |> Level.number |> string_of_int,
                         )
                         |> str}
                      </option>
                    )
                 |> React.array}
              </select>
              <div
                className="pointer-events-none absolute inset-y-0 right-0 flex items-center px-3 text-gray-800">
                <i className="fas fa-chevron-down text-xs" />
              </div>
            </div>
            <button
              title={t("edit_selected_level")}
              ariaLabel={t("edit_selected_level")}
              className="flex items-center text-gray-600 hover:text-gray-900 text-sm font-bold border border-gray-300 border-l-0 py-1 px-2 rounded-r focus:outline-none focus:text-gray-900 focus:ring-2 focus:ring-inset focus:ring-focusColor-500"
              onClick={_ =>
                send(
                  UpdateEditorAction(
                    ShowLevelEditor(Some(state.selectedLevel)),
                  ),
                )
              }>
              <i className="fas fa-pencil-alt" />
            </button>
            <button
              className="btn btn-primary ml-4"
              onClick={_ => send(UpdateEditorAction(ShowLevelEditor(None)))}>
              <i className="fas fa-plus-square mr-2 text-lg" />
              <span> {t("create_level") |> str} </span>
            </button>
          </div>
          {if (showArchivedButton(targetGroupsInLevel, state.targets)) {
             <button
               className="btn btn-default"
               onClick={_ => send(ToggleShowArchived)}>
               {(
                  if (state.showArchived) {
                    "Hide Archived";
                  } else {
                    "Show Archived";
                  }
                )
                |> str}
             </button>;
           } else {
             React.null;
           }}
        </div>
      </div>
      <div className="target-group__container max-w-3xl mt-5 mx-auto relative">
        {targetGroupsToDisplay
         |> Js.Array.mapi(~f=(targetGroup, index) =>
              <CurriculumEditor__TargetGroupShow
                key={targetGroup |> TargetGroup.id}
                targetGroup
                targetGroups=targetGroupsToDisplay
                targets={state.targets}
                showTargetGroupEditorCB
                updateTargetCB
                showArchived={state.showArchived}
                updateTargetSortIndexCB={updateTargetSortIndex(state, send)}
                updateTargetGroupSortIndexCB={updateTargetGroupSortIndex(
                  state,
                  send,
                )}
                index
                course
              />
            )
         |> React.array}
        <button
          ariaLabel={t("create_target_group")}
          onClick={_ =>
            send(UpdateEditorAction(ShowTargetGroupEditor(None)))
          }
          className="target-group__create w-full flex flex-col items-center justify-center relative bg-white border-2 border-dashed border-gray-300 p-6 z-10 hover:text-primary-500 hover:shadow-lg hover:border-primary-400 focus:outline-none focus:text-primary-500 focus:shadow-lg focus:border-primary-400 rounded-lg mt-12 cursor-pointer">
          <span className="flex bg-gray-50 p-2 rounded-full">
            <i className="fas fa-plus-circle text-2xl" />
          </span>
          <h4 className="font-semibold ml-2">
            {t("create_target_group") |> str}
          </h4>
        </button>
      </div>
    </div>
  </div>;
};
