type nonrec file = {
  id: string,
  name: string,
};
type result =
  | Files(array(file))
  | Link(string)
  | ShortText(string)
  | LongText(string)
  | MultiChoice(choices, option(int))
  | AudioRecord(file)
and choices = array(string);
type nonrec t = {
  title: string,
  optional: bool,
  result,
};
let title = t => t.title;
let result = t => t.result;
let optional = t => t.optional;
let make = (~result, ~title, ~optional) => {result, title, optional};
let fromTargetChecklistItem = targetChecklist =>
  targetChecklist
  |> Array.map(tc => {
       let title = tc |> TargetChecklistItem.title;
       let optional = tc |> TargetChecklistItem.optional;
       let result =
         switch (tc |> TargetChecklistItem.kind) {
         | Files => Files([||])
         | Link => Link("")
         | ShortText => ShortText("")
         | LongText => LongText("")
         | MultiChoice(choices) => MultiChoice(choices, None)
         | AudioRecord => AudioRecord({id: "", name: ""})
         };
       make(~title, ~optional, ~result);
     });
let updateResultAtIndex = (index, result, checklist) =>
  checklist
  |> Js.Array.mapi(~f=(c, i) =>
       if (i == index) {
         {...c, result};
       } else {
         c;
       }
     );
let makeFile = (id, name) => {id, name};
let filename = file => file.name;
let fileId = file => file.id;
let fileIds = checklist =>
  checklist
  |> Js.Array.map(~f=c =>
       switch (c.result) {
       | Files(files) => files |> Js.Array.map(~f=a => a.id)
       | AudioRecord(file) => [|file.id|]
       | _anyOtherResult => [||]
       }
     )
  |> ArrayUtils.flattenV2;
let kindAsString = t =>
  switch (t.result) {
  | Files(_) => "files"
  | Link(_) => "link"
  | ShortText(_) => "shortText"
  | LongText(_) => "longText"
  | MultiChoice(_, _) => "multiChoice"
  | AudioRecord(_) => "audio"
  };
let resultAsJson = t =>
  Json.Encode.(
    switch (t.result) {
    | Files(files) => Js.Array.map(~f=file => file.id, files)->stringArray
    | Link(t)
    | ShortText(t)
    | LongText(t) => t->string
    | AudioRecord(file) => file.id->string
    | MultiChoice(choices, index) =>
      string(
        index
        |> OptionUtils.flatMap(i => choices |> ArrayUtils.getOpt(i))
        |> OptionUtils.default(""),
      )
    }
  );
let validString = (s, maxLength) => {
  let length = Js.String.trim(s) |> Js.String.length;
  length >= 1 && length <= maxLength;
};
let validShortText = s => validString(s, 250);
let validLongText = s => validString(s, 5000);
let validFiles = files => files != [||] && files |> Array.length < 4;
let validMultiChoice = (choices, index) =>
  index |> OptionUtils.mapWithDefault(i => choices |> Array.length > i, false);
let validResponse = (response, allowBlank) => {
  let optional =
    if (allowBlank) {
      response.optional;
    } else {
      false;
    };
  switch (response.result, optional) {
  | (Files(files), false) => validFiles(files)
  | (Files(files), true) => files |> ArrayUtils.isEmpty || validFiles(files)
  | (Link(link), false) => link |> UrlUtils.isValid(false)
  | (Link(link), true) => link |> UrlUtils.isValid(true)
  | (ShortText(t), false) => validShortText(t)
  | (ShortText(t), true) => validShortText(t) || t == ""
  | (LongText(t), false) => validLongText(t)
  | (LongText(t), true) => validLongText(t) || t == ""
  | (MultiChoice(choices, index), false) => validMultiChoice(choices, index)
  | (MultiChoice(choices, index), true) =>
    validMultiChoice(choices, index) || index == None
  | (AudioRecord(_), true) => true
  | (AudioRecord(file), false) => file.id != ""
  };
};
let validChecklist = checklist =>
  checklist
  |> Js.Array.map(~f=c => validResponse(c, true))
  |> Js.Array.filter(~f=c => !c)
  |> ArrayUtils.isEmpty;
let validResponses = responses =>
  responses |> Js.Array.filter(~f=c => validResponse(c, false));
let encode = t =>
  Json.Encode.(
    object_([
      ("title", t.title |> string),
      ("kind", kindAsString(t) |> string),
      ("status", "noAnswer" |> string),
      ("result", resultAsJson(t)),
    ])
  );
let encodeArray = checklist =>
  validResponses(checklist) |> Json.Encode.(array(encode));
let makeFiles = checklist =>
  checklist
  |> Js.Array.map(~f=item =>
       switch (item.result) {
       | Files(files) => files
       | AudioRecord(file) => [|file|]
       | _nonFileItem => [||]
       }
     )
  |> ArrayUtils.flattenV2
  |> Array.map(f => {
       let url = "/timeline_event_files/" ++ f.id ++ "/download";
       SubmissionChecklistItem.makeFile(~name=f.name, ~id=f.id, ~url);
     });
