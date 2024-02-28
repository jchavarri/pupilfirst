type nonrec t = {
  id: string,
  number: int,
  filterName: string,
  studentsInLevel: int,
  unlocked: bool,
};
let id = t => t.id;
let number = t => t.number;
let studentsInLevel = t => t.studentsInLevel;
let unlocked = t => t.unlocked;
let filterName = t => t.filterName;
let percentageStudents = (t, totalStudents) =>
  if (totalStudents == 0) {
    0.0;
  } else {
    float_of_int(t.studentsInLevel) /. float_of_int(totalStudents) *. 100.0;
  };
let make = (~id, ~number, ~studentsInLevel, ~unlocked, ~filterName) => {
  id,
  number,
  studentsInLevel,
  unlocked,
  filterName,
};
let sort = levels =>
  levels |> ArrayUtils.copyAndSort((x, y) => x.number - y.number);
let levelsCompletedByAllStudents = levels => {
  let rec aux = (completedLevels, levels) =>
    switch (levels) {
    | [] => completedLevels
    | [head, ...tail] =>
      if (head.studentsInLevel == 0) {
        aux(Array.append(completedLevels, [|head|]), tail);
      } else {
        completedLevels;
      }
    };
  let ls = levels |> sort |> Array.to_list |> aux([||]);

  if (ls |> Array.length == (levels |> Array.length)) {
    [||];
  } else {
    ls;
  };
};
let shortName = t =>
  LevelLabel.format(~short=true, t.number |> string_of_int);
let decode = json =>
  Json.Decode.(
    make(
      ~id=field("id", string, json),
      ~number=field("number", int, json),
      ~studentsInLevel=field("studentsInLevel", int, json),
      ~unlocked=field("unlocked", bool, json),
      ~filterName=field("filterName", string, json),
    )
  );