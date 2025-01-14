exception UnexpectedProgressionBehavior(string);
module Image = {
  type nonrec t = {
    url: string,
    filename: string,
  };
  let url = t => t.url;
  let filename = t => t.filename;
  let make = (url, filename) => {url, filename};
  let decode = json =>
    Json.Decode.{
      url: field("url", string, json),
      filename: field("filename", string, json),
    };
};
module Highlight = {
  type nonrec t = {
    title: string,
    icon: string,
    description: string,
  };
  let title = t => t.title;
  let icon = t => t.icon;
  let description = t => t.description;
  let make = (title, description, icon) => {title, description, icon};
  let updateTitle = (t, title) => {...t, title};
  let updateDescription = (t, description) => {...t, description};
  let updateIcon = (t, icon) => {...t, icon};
  let decode = json =>
    Json.Decode.{
      icon: field("icon", string, json),
      title: field("title", string, json),
      description: field("description", string, json),
    };
  let empty = () => {icon: "badge-check-solid", title: "", description: ""};
  let isInvalid = t =>
    (t.icon == "" || Js.String.trim(t.description) == "")
    || Js.String.trim(t.title) == "";
  let isInValidArray = array => Js.Array.some(~f=isInvalid, array);
};
type nonrec progressionBehavior =
  | Limited(int)
  | Unlimited
  | Strict;
type nonrec t = {
  id: string,
  name: string,
  description: string,
  about: option(string),
  publicSignup: bool,
  publicPreview: bool,
  thumbnail: option(Image.t),
  cover: option(Image.t),
  featured: bool,
  progressionBehavior,
  archivedAt: option(Js.Date.t),
  highlights: array(Highlight.t),
  processingUrl: option(string),
  coachesCount: int,
  levelsCount: int,
  cohortsCount: int,
  defaultCohort: option(Cohort.t),
};
let name = t => t.name;
let id = t => t.id;
let about = t => t.about;
let publicSignup = t => t.publicSignup;
let publicPreview = t => t.publicPreview;
let description = t => t.description;
let featured = t => t.featured;
let cover = t => t.cover;
let thumbnail = t => t.thumbnail;
let archivedAt = t => t.archivedAt;
let highlights = t => t.highlights;
let processingUrl = t => t.processingUrl;
let levelsCount = t => t.levelsCount;
let cohortsCount = t => t.cohortsCount;
let coachesCount = t => t.coachesCount;
let defaultCohort = t => t.defaultCohort;
let progressionBehavior = t =>
  switch (t.progressionBehavior) {
  | Limited(_) => `Limited
  | Unlimited => `Unlimited
  | Strict => `Strict
  };
let progressionLimit = t =>
  switch (t.progressionBehavior) {
  | Limited(limit) => Some(limit)
  | Unlimited
  | Strict => None
  };
let imageUrl = image => image |> Image.url;
let filename = image => image |> Image.filename;
let updateList = (course, courses) =>
  Js.Array.map(
    ~f=
      c =>
        if (c.id == course.id) {
          course;
        } else {
          c;
        },
    courses,
  );
let makeImageFromJs = data =>
  switch (data) {
  | Some(image) => Some(Image.make(image##url, image##filename))
  | None => None
  };
let addImages =
    (~coverUrl, ~thumbnailUrl, ~coverFilename, ~thumbnailFilename, t) => {
  ...t,
  cover:
    switch (coverUrl) {
    | Some(coverUrl) => Some(Image.make(coverUrl, coverFilename))
    | None => None
    },
  thumbnail:
    switch (thumbnailUrl) {
    | Some(thumbnailUrl) =>
      Some(Image.make(thumbnailUrl, thumbnailFilename))
    | None => None
    },
};
let replaceImages = (cover, thumbnail, t) => {...t, cover, thumbnail};
module Fragment = [%graphql
  {js|
  fragment CourseFragment on Course {
    id
    name
    description
    about
    publicSignup
    publicPreview
    thumbnail {
      url
      filename
    }
    cover {
      url
      filename
    }
    featured
    progressionBehavior
    progressionLimit
    archivedAt
    highlights{
      icon
      title
      description
    }
    processingUrl
    coachesCount
    levelsCount
    cohortsCount
    defaultCohort {
      id
      name
      description
      endsAt
      courseId
    }
  }
  |js}
];
let makeFromFragment = (course: Fragment.t) => {
  let archivedAt = Belt.Option.map(course.archivedAt, DateFns.decodeISO);
  let progressionBehavior =
    switch (course.progressionBehavior) {
    | `Limited => Limited(course.progressionLimit |> Belt.Option.getExn)
    | `Unlimited => Unlimited
    | `Strict => Strict
    | `FutureAddedValue(string) =>
      Rollbar.error(
        "Unexpected progression Behavior encountered: " ++ string,
      );
      raise(UnexpectedProgressionBehavior(string));
    };
  {
    id: course.id,
    name: course.name,
    description: course.description,
    about: course.about,
    publicSignup: course.publicSignup,
    publicPreview: course.publicPreview,
    thumbnail:
      course.thumbnail
      ->(Belt.Option.map(image => Image.make(image.url, image.filename))),
    cover:
      course.cover
      ->(Belt.Option.map(image => Image.make(image.url, image.filename))),
    featured: course.featured,
    progressionBehavior,
    archivedAt,
    highlights:
      course.highlights
      ->(
          Js.Array.map(~f=(c: Fragment.t_highlights) =>
            Highlight.make(c.title, c.description, c.icon)
          )
        ),
    processingUrl: course.processingUrl,
    coachesCount: course.coachesCount,
    levelsCount: course.levelsCount,
    cohortsCount: course.cohortsCount,
    defaultCohort:
      course.defaultCohort
      ->(
          Belt.Option.map(cohort =>
            Cohort.make(
              ~id=cohort.id,
              ~name=cohort.name,
              ~description=cohort.description,
              ~endsAt=cohort.endsAt->(Belt.Option.map(DateFns.decodeISO)),
              ~courseId=cohort.courseId,
            )
          )
        ),
  };
};
