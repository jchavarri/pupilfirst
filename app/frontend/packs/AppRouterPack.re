open AppRouter__Types;
let decodeProps = json =>
  Json.Decode.(
    field("courses", array(Course.decode), json),
    field("currentUser", optional(User.decode), json),
    field("school", School.decode, json),
  );
Psj.matchPaths(
  [|
    "courses/:id/review",
    "submissions/:id/review",
    "courses/:id/students",
    "students/:id/report",
  |],
  () => {
    let (courses, currentUser, school) =
      DomUtils.parseJSONTag(~id="app-router-data", ())->decodeProps;
    switch (ReactDOM.querySelector("#app-router")) {
    | Some(root) =>
      let root = ReactDOM.Client.createRoot(root);
      ReactDOM.Client.render(root, <AppRouter school courses currentUser />);
    | None => ()
    };
  },
);
