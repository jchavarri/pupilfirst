type nonrec key = string;
type nonrec value = string;
[@warning "-69"]
type nonrec options = {
  count: option(int),
  defaults: array(Js.Dict.t(string)),
};
type nonrec i18n;
[@mel.module "~/shared/i18n"] external i18n: i18n = "default";
[@mel.send] external translate: (i18n, string, Js.t('a)) => string = "t";
external optionsToJsObject: options => Js.t('a) = "%identity";
external variablesToJsObject: Js.Dict.t(string) => Js.t('a) = "%identity";
let mergeOptionsAndVariables = (options, variables) => {
  let optionsObject = optionsToJsObject(options);
  let variablesObject = variablesToJsObject(variables);
  Js.Obj.assign(optionsObject, variablesObject);
};
let options = (~count, identifier) => {
  let defaultScope =
    Js.Dict.fromArray([|("scope", "shared." ++ identifier)|]);
  {count, defaults: [|defaultScope|]};
};
let t =
    (~scope=?, ~variables: array((key, value))=[||], ~count=?, identifier) => {
  let fullOptions =
    mergeOptionsAndVariables(
      options(~count, identifier),
      Js.Dict.fromArray(variables),
    );
  let fullIdentifier =
    switch (scope) {
    | Some(scope) => scope ++ "." ++ identifier
    | None => identifier
    };
  translate(i18n, fullIdentifier, fullOptions);
};
let ts = (~variables: array((key, value))=[||], ~count=?, identifier) =>
  t(~scope="shared", ~variables, ~count?, identifier);
