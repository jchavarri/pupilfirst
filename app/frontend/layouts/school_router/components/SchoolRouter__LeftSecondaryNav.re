open SchoolRouter__Types;
let str = React.string;
let showLink = (selectedPage, selectedCourse, page, classes, title, contents) => {
  let courseId = selectedCourse->(Belt.Option.map(Course.id));
  let disabled = courseId->Belt.Option.isNone;

  if (Page.path(~courseId?, page) != "#") {
    if (Page.useSPA(selectedPage, page)) {
      <Link
        disabled href={Page.path(~courseId?, page)} className=classes ?title>
        contents
      </Link>;
    } else {
      <a disabled href={Page.path(~courseId?, page)} className=classes ?title>
        contents
      </a>;
    };
  } else {
    SkeletonLoading.secondaryLink();
  };
};
let secondaryNavOption = (selectedPage, selectedCourse, page) => {
  let defaultClasses = "flex text-sm py-3 px-4 hover:bg-gray-50 hover:text-primary-500 focus:bg-gray-50 focus:text-primary-500 rounded items-center my-1";
  let classes =
    defaultClasses
    ++ (
      if (selectedPage == page) {
        " bg-primary-50 text-primary-500 font-semibold";
      } else {
        " font-medium text-gray-500";
      }
    );
  <div key={Page.name(page)}>
    {showLink(
       selectedPage,
       selectedCourse,
       page,
       classes,
       None,
       Page.name(page)->str,
     )}
  </div>;
};
let secondaryNavLinks = (selectedPage, selectedCourse, currentUser) => {
  let navOptionsAdmin = [|
    Page.Curriculum,
    Cohorts,
    Students,
    Applicants,
    Teams,
    Authors,
    Certificates,
    CourseCoaches,
    EvaluationCriteria,
    CourseExports,
  |];
  let navOptionsAuthor = [|Page.Curriculum, EvaluationCriteria|];

  (
    if (User.isAuthor(currentUser)) {
      navOptionsAuthor;
    } else {
      navOptionsAdmin;
    }
  )
  ->(
      Js.Array.map(~f=page =>
        secondaryNavOption(
          selectedPage,
          selectedCourse,
          SelectedCourse(page),
        )
      )
    );
};
[@react.component]
let make = (~selectedPage, ~selectedCourse, ~currentUser) =>
  switch (selectedPage) {
  | Page.Settings(_settingsSelection) =>
    <div
      key="secondary-nav"
      className="bg-white school-admin-navbar__secondary-nav border-r border-gray-200 pb-6 overflow-y-auto">
      <div className="p-4">
        {secondaryNavOption(
           selectedPage,
           selectedCourse,
           Page.Settings(Customization),
         )}
        {secondaryNavOption(
           selectedPage,
           selectedCourse,
           Page.Settings(Admins),
         )}
      </div>
    </div>
  | SelectedCourse(_courseSelection) =>
    <div
      key="secondary-nav"
      className="bg-white school-admin-navbar__secondary-nav border-r border-gray-200 pb-6 overflow-y-auto">
      <div>
        <div className="px-4 pt-2 bg-white">
          {secondaryNavLinks(selectedPage, selectedCourse, currentUser)
           ->React.array}
        </div>
      </div>
    </div>
  | _ => React.null
  };
