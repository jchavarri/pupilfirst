open CoursesReview__Types;

type nonrec selectionItem = {
  itemIndex: int,
  resultIndex: int,
};

let str = React.string;

let t = I18n.t(~scope="components.CoursesReview__ChecklistShow");

let selectChecklist = (itemIndex, resultIndex, setSelecton) =>
  setSelecton(selection =>
    Js.Array.concat(selection, ~other=[|{itemIndex, resultIndex}|])
  );

let unSelectChecklist = (itemIndex, resultIndex, setSelecton) =>
  setSelecton(selection =>
    Js.Array.filter(
      ~f=
        item =>
          !(item.itemIndex == itemIndex && item.resultIndex == resultIndex),
      selection,
    )
  );

let checkboxOnChange = (itemIndex, resultIndex, setSelecton, event) =>
  if (React.Event.Form.target(event)##checked) {
    selectChecklist(itemIndex, resultIndex, setSelecton);
  } else {
    unSelectChecklist(itemIndex, resultIndex, setSelecton);
  };

let generateFeedback =
    (checklist, selection, feedback, setSelecton, updateFeedbackCB) => {
  let newFeedback =
    feedback
    ++ (
      if (String.trim(feedback) == "") {
        "";
      } else {
        "\\n\\n";
      }
    )
    ++ checklist
       ->(
           Js.Array.mapi(~f=(reviewChecklistItem, i) => {
             let resultIndexList =
               selection
               ->(
                   Js.Array.filter(~f=selectionItem =>
                     selectionItem.itemIndex == i
                   )
                 )
               ->(Js.Array.map(~f=item => item.resultIndex));

             ReviewChecklistItem.result(reviewChecklistItem)
             ->(
                 Js.Array.mapi(~f=(resultItem, index) =>
                   if (resultIndexList->(Js.Array.some(~f=i => i == index))) {
                     switch (ReviewChecklistResult.feedback(resultItem)) {
                     | Some(feedback) => [|feedback|]
                     | None => [||]
                     };
                   } else {
                     [||];
                   }
                 )
               )
             ->ArrayUtils.flattenV2;
           })
         )
       ->ArrayUtils.flattenV2
       ->(Js.Array.join(~sep="\\n\\n"));
  setSelecton(_ => [||]);
  updateFeedbackCB(newFeedback);
};
let checklistItemCheckedClasses = (itemIndex, selection) =>
  "absolute w-1 inset-0 rounded-r-md "
  ++ (
    if (Js.Array.filter(~f=s => s.itemIndex == itemIndex, selection)
        ->ArrayUtils.isNotEmpty) {
      "bg-green-400";
    } else {
      "bg-gray-500";
    }
  );

let feedbackGeneratable = (submissionDetails, overlaySubmission) =>
  SubmissionDetails.reviewer(submissionDetails)->Belt.Option.isSome
  || OverlaySubmission.evaluatedAt(overlaySubmission)->Belt.Option.isSome;

let checklistItemChecked = (itemIndex, resultIndex, selection) =>
  Js.Array.filter(
    ~f=s => s.itemIndex == itemIndex && s.resultIndex == resultIndex,
    selection,
  )
  ->ArrayUtils.isNotEmpty;

let updateChecklistResultFeedback =
    (
      itemIndex,
      resultIndex,
      feedback,
      reviewChecklistItem,
      resultItem,
      setChecklist,
    ) => {
  let newReviewChecklistItem =
    ReviewChecklistItem.updateChecklist(
      ReviewChecklistResult.updateFeedback(
        feedback,
        resultItem,
        resultIndex,
        ReviewChecklistItem.result(reviewChecklistItem),
      ),
      reviewChecklistItem,
    );

  setChecklist(checklist =>
    ReviewChecklistItem.replace(newReviewChecklistItem, itemIndex, checklist)
  );
};

let generateFeedbackButton =
    (checklist, selection, feedback, setSelecton, updateFeedbackCB) =>
  <button
    className="btn btn-primary w-full md:w-auto"
    disabled={selection->ArrayUtils.isEmpty}
    onClick={_ =>
      generateFeedback(
        checklist,
        selection,
        feedback,
        setSelecton,
        updateFeedbackCB,
      )
    }>
    {t("generate_feedback_button")->str}
  </button>;

[@react.component]
let make =
    (
      ~reviewChecklist,
      ~feedback,
      ~updateFeedbackCB,
      ~showEditorCB,
      ~cancelCB,
      ~overlaySubmission,
      ~submissionDetails,
    ) => {
  let (checklist, setChecklist) = React.useState(() => reviewChecklist);
  let (selection, setSelecton) = React.useState(() => [||]);
  let (id, _setId) =
    React.useState(() => DateTime.randomId() ++ "-review-checkbox-");

  <div>
    <div
      className="flex items-center px-4 md:px-6 py-3 bg-white border-b sticky top-0 z-50 h-16">
      <div className="flex flex-1 items-center justify-between">
        <button
          className="btn btn-subtle focus:ring-2 focus:ring-offset-2 focus:ring-focusColor-500 transition"
          onClick=cancelCB>
          <FaIcon classes="fas fa-arrow-left text-gray-500" />
          <p className="pl-2"> {str("Back to Review")} </p>
        </button>
      </div>
    </div>
    <div className="p-4 md:px-6 pb-0">
      <div className="flex items-end justify-between">
        <h5 className="font-semibold flex items-center tracking-wide text-sm">
          {t("review_checklist")->str}
        </h5>
        <button
          className="btn btn-small btn-default" onClick={_ => showEditorCB()}>
          <i className="far fa-edit" />
          <div className="ml-2"> {t("edit_checklist_button")->str} </div>
        </button>
      </div>
      <div className="border bg-white rounded-lg py-2 md:py-4 mt-2 space-y-8">
        {Js.Array.mapi(
           ~f=
             (reviewChecklistItem, itemIndex) =>
               <Spread
                 props={"data-checklist-item": string_of_int(itemIndex)}
                 key={string_of_int(itemIndex)}>
                 <div>
                   <h4
                     className="relative text-sm font-semibold mt-2 md:mt-0 px-6 w-full md:w-4/5">
                     <div
                       className={checklistItemCheckedClasses(
                         itemIndex,
                         selection,
                       )}
                     />
                     {ReviewChecklistItem.title(reviewChecklistItem)->str}
                   </h4>
                   <div className="space-y-3 pt-3">
                     {Js.Array.mapi(
                        ~f=
                          (checklistItem, resultIndex) =>
                            <Spread
                              props={
                                "data-result-item":
                                  string_of_int(resultIndex),
                              }
                              key={
                                string_of_int(itemIndex)
                                ++ string_of_int(resultIndex)
                              }>
                              <div className="px-6">
                                <Checkbox
                                  id={
                                    id
                                    ++ itemIndex->string_of_int
                                    ++ resultIndex->string_of_int
                                  }
                                  label={str(
                                    checklistItem->ReviewChecklistResult.title,
                                  )}
                                  onChange={checkboxOnChange(
                                    itemIndex,
                                    resultIndex,
                                    setSelecton,
                                  )}
                                  checked={checklistItemChecked(
                                    itemIndex,
                                    resultIndex,
                                    selection,
                                  )}
                                  disabled={
                                    !
                                      feedbackGeneratable(
                                        submissionDetails,
                                        overlaySubmission,
                                      )
                                  }
                                />
                                {let isSelected =
                                   Js.Array.find(
                                     ~f=
                                       s =>
                                         s.itemIndex == itemIndex
                                         && s.resultIndex == resultIndex,
                                     selection,
                                   )
                                   ->Belt.Option.isSome;

                                 ReactUtils.nullUnless(
                                   <div className="pl-7 pt-2">
                                     <textarea
                                       rows=4
                                       cols=33
                                       className="appearance-none border border-gray-300 bg-white rounded-b text-sm align-top py-2 px-4 leading-relaxed w-full focus:outline-none focus:bg-white focus:border-primary-300"
                                       id={
                                         "result_"
                                         ++ resultIndex->string_of_int
                                         ++ "_feedback"
                                       }
                                       type_="text"
                                       placeholder={t("feedback_placeholder")}
                                       disabled={
                                         !
                                           feedbackGeneratable(
                                             submissionDetails,
                                             overlaySubmission,
                                           )
                                       }
                                       value={Belt.Option.getWithDefault(
                                         ReviewChecklistResult.feedback(
                                           checklistItem,
                                         ),
                                         "",
                                       )}
                                       onChange={event =>
                                         updateChecklistResultFeedback(
                                           itemIndex,
                                           resultIndex,
                                           React.Event.Form.target(event)##value,
                                           reviewChecklistItem,
                                           checklistItem,
                                           setChecklist,
                                         )
                                       }
                                     />
                                   </div>,
                                   isSelected
                                   || !
                                        feedbackGeneratable(
                                          submissionDetails,
                                          overlaySubmission,
                                        ),
                                 )}
                              </div>
                            </Spread>,
                        ReviewChecklistItem.result(reviewChecklistItem),
                      )
                      ->React.array}
                   </div>
                 </div>
               </Spread>,
           checklist,
         )
         ->React.array}
      </div>
    </div>
    <div
      className="flex justify-end border-t bg-gray-50 opacity-90 sticky bottom-0 px-4 md:px-6 py-2 md:py-4 mt-4">
      {if (feedbackGeneratable(submissionDetails, overlaySubmission)) {
         generateFeedbackButton(
           checklist,
           selection,
           feedback,
           setSelecton,
           updateFeedbackCB,
         );
       } else {
         React.null;
       }}
    </div>
  </div>;
};
