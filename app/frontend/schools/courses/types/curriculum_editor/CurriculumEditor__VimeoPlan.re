exception UnknownPlanName(string);
type nonrec t =
  | Basic
  | Plus
  | Pro
  | Business
  | Premium;
let decode = planName =>
  switch (planName) {
  | "basic" => Basic
  | "plus" => Plus
  | "pro" => Pro
  | "business" => Business
  | "premium" => Premium
  | unknownPlan => raise(UnknownPlanName(unknownPlan))
  };
