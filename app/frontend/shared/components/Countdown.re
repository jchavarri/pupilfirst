let str = React.string;
type nonrec state = {
  seconds: int,
  timeoutId: option(Js.Global.timeoutId),
  reload: bool,
};
type nonrec action =
  | SetTimeout(Js.Global.timeoutId)
  | Decrement
  | ToggleReload;
let reducer = (state, action) =>
  switch (action) {
  | ToggleReload => {...state, reload: !state.reload}
  | SetTimeout(timeoutId) => {...state, timeoutId: Some(timeoutId)}
  | Decrement => {...state, seconds: state.seconds - 1, reload: !state.reload}
  };
let percentage = (current, total) =>
  int_of_float(float_of_int(current) /. float_of_int(total) *. 100.00);
let reloadTimer = (timeoutCB, state, send, ()) => {
  state.timeoutId->(Belt.Option.forEach(Js.Global.clearTimeout));

  if (state.seconds == 0) {
    timeoutCB();
  } else {
    send(Decrement);
  };
};
let reload = (timeoutCB, state, send, ()) => {
  let timeoutId =
    Js.Global.setTimeout(
      ~f=reloadTimer(timeoutCB, state, send),
      state.timeoutId->(Belt.Option.mapWithDefault(0, _ => 1000)),
    );
  send(SetTimeout(timeoutId));
  None;
};
[@react.component]
let make = (~seconds, ~timeoutCB) => {
  let (state, send) =
    React.useReducer(reducer, {seconds, timeoutId: None, reload: false});
  React.useEffect1(reload(timeoutCB, state, send), [|state.reload|]);
  <div>
    <DoughnutChart
      mode={
             if (state.seconds == 0) {
               DoughnutChart.Indeterminate;
             } else {
               DoughnutChart.Determinate(state.seconds, seconds);
             }
           }
      hideSymbol=true
      className="mx-auto"
    />
  </div>;
};
