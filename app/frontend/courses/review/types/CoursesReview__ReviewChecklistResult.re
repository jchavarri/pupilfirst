type nonrec t = {
  title: string,
  feedback: option(string),
};
let title = t => t.title;
let feedback = t => t.feedback;
let make = (~title, ~feedback) => {title, feedback};
let makeFromJs = data =>
  data |> Js.Array.map(~f=r => make(~title=r##title, ~feedback=r##feedback));
let emptyTemplate = () => [|
  make(~title="Yes", ~feedback=Some("Sample feedback for yes")),
  make(~title="No", ~feedback=Some("Sample feedback for no")),
|];
let empty = () => make(~title="", ~feedback=None);
let replace = (t, index, checklist) =>
  checklist
  |> Array.mapi((resultIndex, result) =>
       if (resultIndex == index) {
         t;
       } else {
         result;
       }
     );
let updateTitle = (title, t, index, checklist) =>
  checklist |> replace(make(~title, ~feedback=t.feedback), index);
let updateFeedback = (feedback, t, index, checklist) => {
  let optionalFeedback =
    if (feedback |> Js.String.trim == "") {
      None;
    } else {
      Some(feedback);
    };
  checklist
  |> replace(make(~title=t.title, ~feedback=optionalFeedback), index);
};
let trim = t => {...t, title: t.title |> String.trim};
let encode = t => {
  let title = [("title", t.title |> Json.Encode.string)];
  let feedback =
    switch (t.feedback) {
    | Some(f) => [("feedback", f |> Json.Encode.string)]
    | None => []
    };
  Json.Encode.(object_(title @ feedback));
};
