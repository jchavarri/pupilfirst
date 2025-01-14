let initials = name =>
  name
  |> Js.String.split(~sep=" ")
  |> Js.Array.slice(~start=0, ~end_=2)
  |> Js.Array.map(~f=word => word |> Js.String.slice(~start=0, ~end_=1))
  |> Js.Array.join(~sep="");
[@react.component]
let make = (~colors=?, ~name, ~className=?) => {
  let (bgColor, fgColor) =
    switch (colors) {
    | Some((bgColor, fgColor)) => (bgColor, fgColor)
    | None => StringUtils.toColor(name)
    };
  <svg
    xmlns="http://www.w3.org/2000/svg"
    version="1.1"
    viewBox="0 0 100 100"
    title=name
    ?className>
    <circle cx="50" cy="50" r="50" fill=bgColor />
    <text
      fill=fgColor
      fontSize="42"
      fontFamily="sans-serif"
      x="50"
      y="54"
      textAnchor="middle"
      dominantBaseline="middle"
      alignmentBaseline="middle">
      {initials(name) |> React.string}
    </text>
  </svg>;
};
let makeFromJson = json =>
  Json.Decode.(
    make(
      [%obj
        {
          colors: optional(field("id", pair(string, string)), json),
          name: field("name", string, json),
          className: optional(field("placeholder", string), json),
        }
      ],
    )
  );
