let str = React.string;

let t = I18n.t(~scope="components.StudentsIndex__Root");

open StudentsIndex__Types;

module Item = {
  type nonrec t = StudentInfo.t;
};

module PagedStudents = Pagination.Make(Item);

type nonrec state = {
  loading: LoadingV2.t,
  students: PagedStudents.t,
  levels: array(Level.t),
  totalEntriesCount: int,
};

type nonrec action =
  | LoadStudents(option(string), bool, array(StudentInfo.t), int)
  | BeginLoadingMore
  | BeginReloading;

let reducer = (state, action) =>
  switch (action) {
  | LoadStudents(endCursor, hasNextPage, students, totalEntriesCount) =>
    let updatedStudent =
      switch (state.loading) {
      | LoadingMore =>
        Js.Array.concat(PagedStudents.toArray(state.students), ~other=students)
      | Reloading(_) => students
      };

    {
      ...state,
      students: PagedStudents.make(updatedStudent, hasNextPage, endCursor),
      loading: LoadingV2.setNotLoading(state.loading),
      totalEntriesCount,
    };
  | BeginLoadingMore => {...state, loading: LoadingMore}
  | BeginReloading => {
      ...state,
      loading: LoadingV2.setReloading(state.loading),
    }
  };

module LevelFragment = Shared__Level.Fragment;
module CohortFragment = Cohort.Fragment;
module AdminUserFragment = Admin__User.Fragment;

module CourseStudentsQuery = [%graphql
  {js|
    query CourseStudentsQuery($courseId: ID!, $after: String, $filterString: String) {
      courseStudents(courseId: $courseId, filterString: $filterString,first: 20, after: $after) {
        nodes {
          id
          taggings
          user {
            ...AdminUserFragment
          }
          level {
            ...LevelFragment
          }
          cohort {
            ...CohortFragment
          }
        }
        pageInfo {
          endCursor,
          hasNextPage
        }
        totalCount
      }
    }
  |js}
];

let getStudents = (send, courseId, cursor, params) => {
  let filterString = Webapi.Url.URLSearchParams.toString(params);

  CourseStudentsQuery.makeVariables(
    ~courseId,
    ~after=?cursor,
    ~filterString=?Some(filterString),
    (),
  )
  |> CourseStudentsQuery.fetch
  |> Js.Promise.then_((response: CourseStudentsQuery.t) => {
       send(
         LoadStudents(
           response.courseStudents.pageInfo.endCursor,
           response.courseStudents.pageInfo.hasNextPage,
           response.courseStudents.nodes
           ->(
               Js.Array.map(~f=(studentDetails: CourseStudentsQuery.t_courseStudents_nodes) =>
                 StudentInfo.make(
                   ~id=studentDetails.id,
                   ~taggings=studentDetails.taggings,
                   ~user=Admin__User.makeFromFragment(studentDetails.user),
                   ~level=
                     Shared__Level.makeFromFragment(studentDetails.level),
                   ~cohort=Cohort.makeFromFragment(studentDetails.cohort),
                 )
               )
             ),
           response.courseStudents.totalCount,
         ),
       );
       Js.Promise.resolve();
     })
  |> ignore;
};

let computeInitialState = () => {
  loading: LoadingV2.empty(),
  students: Unloaded,
  levels: [||],
  totalEntriesCount: 0,
};

let reloadStudents = (courseId, send, params) => {
  send(BeginReloading);
  getStudents(send, courseId, None, params);
};

let onSelect = (key, value, params) => {
  Webapi.Url.URLSearchParams.set(key, value, params);
  ReasonReactRouter.push(
    "?" ++ Webapi.Url.URLSearchParams.toString(params),
  );
};

let showTag = (~value=?, key, text, color, params) => {
  let paramsValue = Belt.Option.getWithDefault(value, text);
  <button
    key=text
    className={
      "rounded-lg mt-1 mr-1 py-px px-2 text-xs font-semibold focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 "
      ++ color
    }
    onClick={_e => onSelect(key, paramsValue, params)}>
    text->str
  </button>;
};

let studentsList = (params, students) =>
  <div className="space-y-4">
    {students
     ->(
         Js.Array.map(~f=student =>
           <div
             key={StudentInfo.id(student)}
             className="student-container h-full flex items-center bg-white">
             <div
               className="py-4 px-4 flex gap-4 flex-1 items-center text-left justify-between rounded-md shadow">
               <div className="flex">
                 <div className="text-sm flex items-center space-x-4">
                   {switch (User.avatarUrl(StudentInfo.user(student))) {
                    | Some(avatarUrl) =>
                      <img
                        className="h-9 w-9 md:h-12 md:w-12 text-xs border border-gray-300 rounded-full overflow-hidden flex-shrink-0 object-cover"
                        src=avatarUrl
                      />
                    | None =>
                      <Avatar
                        name={User.name(StudentInfo.user(student))}
                        className="h-9 w-9 md:h-12 md:w-12 text-xs border border-gray-300 rounded-full overflow-hidden flex-shrink-0 object-cover"
                      />
                    }}
                   <div>
                     <Link
                       href={
                         (
                           {js|/school/students/|js} ++ StudentInfo.id(student)
                         )
                         ++ {js|/details|js}
                       }
                       className="font-semibold inline-block hover:underline hover:text-primary-500 transition ">
                       {User.name(StudentInfo.user(student))->str}
                     </Link>
                     <div className="flex flex-row mt-1">
                       <div className="flex flex-wrap">
                         {showTag(
                            "cohort",
                            Cohort.name(StudentInfo.cohort(student)),
                            "bg-green-100 text-green-900",
                            params,
                            ~value=
                              Cohort.filterValue(
                                StudentInfo.cohort(student),
                              ),
                          )}
                         {showTag(
                            "level",
                            Level.shortName(StudentInfo.level(student)),
                            "bg-yellow-100 text-yellow-900",
                            params,
                            ~value=
                              Level.filterValue(StudentInfo.level(student)),
                          )}
                         {StudentInfo.taggings(student)
                          ->(
                              Js.Array.map(~f=tag =>
                                showTag(
                                  "student_tags",
                                  tag,
                                  "bg-gray-200 text-gray-900",
                                  params,
                                )
                              )
                            )
                          ->React.array}
                         {User.taggings(StudentInfo.user(student))
                          ->(
                              Js.Array.map(~f=tag =>
                                showTag(
                                  "user_tags",
                                  tag,
                                  "bg-blue-100 text-blue-900",
                                  params,
                                )
                              )
                            )
                          ->React.array}
                       </div>
                     </div>
                   </div>
                 </div>
               </div>
               <Spread
                 props={
                   "data-student-name": User.name(StudentInfo.user(student)),
                 }>
                 <div>
                   <Link
                     ariaLabel={
                       (
                         {js|Edit |js} ++ User.name(StudentInfo.user(student))
                       )
                       ++ {js|'s profile|js}
                     }
                     href={
                       ({js|/school/students/|js} ++ StudentInfo.id(student))
                       ++ {js|/details|js}
                     }
                     className="flex flex-1 items-center rounded-md hover:bg-primary-50 hover:text-primary-500 focus:bg-primary-50 focus:text-primary-500 justify-between">
                     <span className="inline-flex items-center p-2">
                       <PfIcon className="if i-edit-regular if-fw" />
                       <span className="ml-2"> {t("edit")->str} </span>
                     </span>
                   </Link>
                 </div>
               </Spread>
             </div>
           </div>
         )
       )
     ->React.array}
  </div>;

let makeFilters = () => [|
  CourseResourcesFilter.makeFilter(
    "cohort",
    t("cohort"),
    DataLoad(`Cohort),
    "green",
  ),
  CourseResourcesFilter.makeFilter(
    "level",
    t("level"),
    DataLoad(`Level),
    "yellow",
  ),
  CourseResourcesFilter.makeFilter(
    "student_tags",
    "Student Tag",
    DataLoad(`StudentTag),
    "focusColor",
  ),
  CourseResourcesFilter.makeFilter(
    "user_tags",
    t("filter.user_tag"),
    DataLoad(`UserTag),
    "blue",
  ),
  CourseResourcesFilter.makeFilter(
    "email",
    t("filter.search_by_email"),
    Search,
    "gray",
  ),
  CourseResourcesFilter.makeFilter(
    "name",
    t("filter.search_by_name"),
    Search,
    "gray",
  ),
  CourseResourcesFilter.makeFilter(
    "status",
    "Status",
    CustomArray([|"Active", "Ended", "Dropped"|]),
    "orange",
  ),
|];

let renderLoadMore = (send, courseId, params, cursor) =>
  <div className="pb-6">
    <button
      className="btn btn-primary-ghost cursor-pointer w-full"
      onClick={_ => {
        send(BeginLoadingMore);
        getStudents(send, courseId, Some(cursor), params);
      }}>
      {t("load_more")->str}
    </button>
  </div>;

[@react.component]
let make = (~courseId, ~search) => {
  let (state, send) = React.useReducer(reducer, computeInitialState());
  let params = Webapi.Url.URLSearchParams.make(search);
  React.useEffect1(
    () => {
      reloadStudents(courseId, send, params);
      None;
    },
    [|search|],
  );

  <>
    <Helmet> <title> {str(t("title"))} </title> </Helmet>
    <div className="bg-gray-50 pt-8 min-h-full">
      <div className="max-w-4xl 2xl:max-w-5xl mx-auto px-4">
        <div className="flex justify-between items-end gap-2">
          <p className="font-semibold pl-1 capitalize">
            {t("students")->str}
          </p>
          <div>
            <Link
              className="btn btn-primary"
              href={
                ({js|/school/courses/|js} ++ courseId) ++ {js|/students/new|js}
              }>
              <PfIcon className="if i-plus-regular" />
              <span className="inline-block pl-2">
                {str(t("add_new_students"))}
              </span>
            </Link>
          </div>
        </div>
        <div className="sticky top-0 my-6">
          <div className="border rounded-lg mx-auto bg-white ">
            <div>
              <div className="flex w-full items-start p-4">
                <CourseResourcesFilter
                  courseId
                  filters={makeFilters()}
                  search
                  sorter={CourseResourcesFilter.makeSorter(
                    "sort_by",
                    [|
                      t("sorter.name"),
                      t("sorter.first_created"),
                      t("sorter.last_created"),
                      t("sorter.first_updated"),
                      t("sorter.last_updated"),
                    |],
                    t("sorter.last_created"),
                  )}
                />
              </div>
            </div>
          </div>
        </div>
        {PagedStudents.renderView(
           ~pagedItems=state.students,
           ~loading=state.loading,
           ~entriesView=studentsList(params),
           ~totalEntriesCount=state.totalEntriesCount,
           ~loadMore=renderLoadMore(send, courseId, params),
           ~singularResourceName=t("student"),
           ~pluralResourceName=t("students"),
           ~emptyMessage=t("pagination.empty_message"),
         )}
      </div>
    </div>
  </>;
};
