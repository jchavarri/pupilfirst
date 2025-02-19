let str = React.string;
let t = I18n.t(~scope="components.TeamsCreator__Root");
[@react.component]
let make = (~courseId) => {
  let courseContext = React.useContext(SchoolRouter__CourseContext.context);
  <div>
    <School__PageHeader
      exitUrl={({js|/school/courses/|js} ++ courseId) ++ {js|/teams|js}}
      title={t("title")}
      description={
        courseContext.selectedCourse
        ->(Belt.Option.mapWithDefault("Course", AppRouter__Course.name))
      }
    />
    <AdminCoursesShared__TeamEditor courseId />
  </div>;
};
