type nonrec t = {
  id: string,
  name: string,
};
let id = t => t.id;
let name = t => t.name;
let decode = json =>
  Json.Decode.{
    id: json |> field("id", string),
    name: json |> field("name", string),
  };
