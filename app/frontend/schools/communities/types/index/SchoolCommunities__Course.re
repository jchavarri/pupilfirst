type nonrec t = {
  id: string,
  name: string,
};
let decode = json =>
  Json.Decode.{
    id: json |> field("id", string),
    name: json |> field("name", string),
  };
let id = t => t.id;
let name = t => t.name;
