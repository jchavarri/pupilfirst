module type Selectable = {
  type nonrec t;
  let name: t => string;
};
module Make = (Selectable: Selectable) => {
  let showSelectables = (selectables, onSelect) =>
    selectables->(
                   Js.Array.mapi(~f=(selectable, index) =>
                     <button
                       key={string_of_int(index)}
                       onClick={_ => onSelect(selectable)}
                       className="w-full text-left cursor-pointer block p-3 bg-white hover:text-primary-500 hover:bg-primary-50 focus:outline-none focus:text-primary-500 focus:bg-primary-50">
                       {Selectable.name(selectable)->React.string}
                     </button>
                   )
                 );
  let showSelected = (placeholder, selected, loading, disabled) =>
    <button
      disabled={loading || disabled}
      className="mt-1 flex items-center justify-between appearance-none w-full bg-white border border-gray-300 rounded py-2.5 px-3 text-sm focus:outline-none focus:bg-white focus:border-transparent focus:ring-2 focus:ring-focusColor-500"
      key="selected">
      <span>
        (
          switch (selected) {
          | Some(s) => Selectable.name(s)
          | None => placeholder
          }
        )
        ->React.string
      </span>
      {if (loading) {
         <FaIcon classes="fas fa-spinner fa-spin" />;
       } else {
         <FaIcon classes="fas fa-caret-down ml-2" />;
       }}
    </button>;
  [@react.component]
  let make =
      (
        ~placeholder,
        ~selectables,
        ~selected,
        ~onSelect,
        ~loading=false,
        ~disabled=false,
      ) =>
    <Dropdown2
      selected={showSelected(placeholder, selected, loading, disabled)}
      contents={showSelectables(selectables, onSelect)}
    />;
};
