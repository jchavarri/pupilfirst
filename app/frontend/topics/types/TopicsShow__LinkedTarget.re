type nonrec t = {
  id: option(string),
  title: string,
};
let decode = json =>
  Json.Decode.{
    id: json |> field("id", nullable(string)) |> Js.Null.toOption,
    title: json |> field("title", string),
  };
let id = t => t.id;
let title = t => t.title;
