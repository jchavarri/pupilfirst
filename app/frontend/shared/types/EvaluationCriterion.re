type nonrec t = {
  name: string,
  id: string,
  maxGrade: int,
  passGrade: int,
  gradesAndLabels: array(GradeLabel.t),
};
let decode = json =>
  Json.Decode.{
    id: json |> field("id", string),
    name: json |> field("name", string),
    gradesAndLabels: json |> field("gradeLabels", array(GradeLabel.decode)),
    maxGrade: json |> field("maxGrade", int),
    passGrade: json |> field("passGrade", int),
  };
let id = t => t.id;
let maxGrade = t => t.maxGrade;
let sort = evaluationCriteria =>
  evaluationCriteria
  |> ArrayUtils.copyAndSort((x, y) =>
       Js.String.localeCompare(x.name, ~other=y.name)->int_of_float
     );
let name = t => t.name;
let gradesAndLabels = t => t.gradesAndLabels;
let passGrade = t => t.passGrade;
let makeFromJs = evaluationCriterion => {
  name: evaluationCriterion##name,
  id: evaluationCriterion##id,
  maxGrade: evaluationCriterion##maxGrade,
  passGrade: evaluationCriterion##passGrade,
  gradesAndLabels:
    evaluationCriterion##gradeLabels
    |> Js.Array.map(~f=gL => gL |> GradeLabel.makeFromJs),
};
let make = (~id, ~name, ~maxGrade, ~passGrade, ~gradesAndLabels) => {
  id,
  name,
  maxGrade,
  passGrade,
  gradesAndLabels,
};
