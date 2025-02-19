[%%raw {js|import "./LevelProgressBar.css"|js}];
let str = React.string;
let t = I18n.t(~scope="components.LevelProgressBar");
let levelClasses = (levelNumber, levelCompleted, currentLevelNumber) => {
  let reached =
    if (levelNumber <= currentLevelNumber) {
      "level-progress-bar__student-level--reached";
    } else {
      "";
    };
  let current =
    if (levelNumber == currentLevelNumber) {
      " level-progress-bar__student-level--current";
    } else {
      "";
    };
  let completed =
    if (levelCompleted) {" level-progress-bar__student-level--completed"} else {
      ""
    };
  reached ++ current ++ completed;
};
[@react.component]
let make = (~levels, ~currentLevelNumber, ~courseCompleted, ~className="") =>
  <div className>
    <div className="flex justify-between items-end">
      <h6 className="text-sm font-semibold"> {t("heading") |> str} </h6>
    </div>
    <div className="h-12 flex items-center">
      <ul
        className={
          "level-progress-bar__student-progress flex w-full "
          ++ (
            if (courseCompleted) {
              "level-progress-bar__student-progress--completed"
            } else {
              ""
            }
          )
        }>
        {levels->(
                   Js.Array.mapi(~f=(levelCompleted, index) => {
                     let levelNumber = index + 1;
                     <li
                       key={Belt.Int.toString(levelNumber)}
                       className={
                         "flex-1 level-progress-bar__student-level "
                         ++ levelClasses(
                              levelNumber,
                              levelCompleted,
                              currentLevelNumber,
                            )
                       }>
                       <span
                         className="level-progress-bar__student-level-count">
                         {levelNumber |> string_of_int |> str}
                       </span>
                     </li>;
                   })
                 )
         |> React.array}
      </ul>
    </div>
  </div>;
let makeFromJson = json =>
  Json.Decode.(
    make(
      [%obj
        {
          levels: field("levels", array(bool), json),
          currentLevelNumber: field("currentLevelNumber", int, json),
          courseCompleted: field("courseCompleted", bool, json),
          className: optional(field("className", string), json),
        }
      ],
    )
  );
