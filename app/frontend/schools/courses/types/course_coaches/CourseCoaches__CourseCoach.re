type nonrec t = {
  id: string,
  name: string,
  email: string,
  avatarUrl: option(string),
  title: string,
};
let name = t => t.name;
let email = t => t.email;
let id = t => t.id;
let avatarUrl = t => t.avatarUrl;
let title = t => t.title;
let decode = json =>
  Json.Decode.{
    name: json |> field("name", string),
    email: json |> field("email", string),
    id: json |> field("id", string),
    avatarUrl: json |> optional(field("avatarUrl", string)),
    title: json |> field("title", string),
  };
