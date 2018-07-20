// Generated by BUCKLESCRIPT VERSION 3.1.5, PLEASE EDIT WITH CARE
'use strict';

var Block = require("bs-platform/lib/js/block.js");
var Curry = require("bs-platform/lib/js/curry.js");
var Fetch = require("bs-fetch/src/Fetch.js");
var React = require("react");
var Json_decode = require("@glennsl/bs-json/src/Json_decode.bs.js");
var ReasonReact = require("reason-react/src/ReasonReact.js");
var Caml_exceptions = require("bs-platform/lib/js/caml_exceptions.js");
var Notification$ReactTemplate = require("../types/Notification.bs.js");
var TimelineEvent$ReactTemplate = require("../types/TimelineEvent.bs.js");
var TrixEditor = require("../../../admin/components/eventsReviewDashboard/TrixEditor");

var UnexpectedResponse = Caml_exceptions.create("FeedbackForm-ReactTemplate.UnexpectedResponse");

function handleApiError(match) {
  if (Caml_exceptions.isCamlExceptionOrOpenVariant(match) && match[0] === UnexpectedResponse) {
    return /* Some */[match[1]];
  } else {
    return /* None */0;
  }
}

function make(onChange, children) {
  return ReasonReact.wrapJsForReason(TrixEditor.default, {
              onChange: onChange
            }, children);
}

var TrixEditor$1 = /* module */[/* make */make];

function str(prim) {
  return prim;
}

function toggleForm(send, _) {
  return Curry._1(send, /* ToggleForm */0);
}

function updateFeedback(send, html) {
  return Curry._1(send, /* UpdateFeedback */[html]);
}

function clearFeedback(send, _) {
  Curry._1(send, /* UpdateFeedback */[""]);
  return Curry._1(send, /* ToggleForm */0);
}

function handleResponseJSON(send, json) {
  var match = Json_decode.field("error", (function (param) {
          return Json_decode.nullable(Json_decode.string, param);
        }), json);
  if (match !== null) {
    return Notification$ReactTemplate.error("Something went wrong!", match);
  } else {
    Notification$ReactTemplate.success("Feedback Sent", "Your feedback has been recorded and emailed to the student(s)");
    Curry._1(send, /* UpdateFeedback */[""]);
    return Curry._1(send, /* ToggleForm */0);
  }
}

function sendFeedback(state, send, te, authenticityToken, _) {
  console.log("Sending feedback for emailing");
  console.log("Feedback to be sent:" + state[/* feedbackHTML */1]);
  var payload = { };
  payload["authenticity_token"] = authenticityToken;
  payload["feedback"] = state[/* feedbackHTML */1];
  var id = String(TimelineEvent$ReactTemplate.id(te));
  fetch("/timeline_events/" + (id + "/send_feedback"), Fetch.RequestInit[/* make */0](/* Some */[/* Post */2], /* Some */[{
                      "Content-Type": "application/json"
                    }], /* Some */[JSON.stringify(payload)], /* None */0, /* None */0, /* None */0, /* Some */[/* SameOrigin */1], /* None */0, /* None */0, /* None */0, /* None */0)(/* () */0)).then((function (response) {
              if (response.ok || response.status === 422) {
                return response.json();
              } else {
                return Promise.reject([
                            UnexpectedResponse,
                            response.status
                          ]);
              }
            })).then((function (json) {
            return Promise.resolve(handleResponseJSON(send, json));
          })).catch((function (error) {
          var match = handleApiError(error);
          return Promise.resolve(match ? Notification$ReactTemplate.error(String(match[0]), "Please try again") : Notification$ReactTemplate.error("Something went wrong!", "Please try again"));
        }));
  return /* () */0;
}

var component = ReasonReact.reducerComponent("FeedbackForm");

function make$1(timelineEvent, authenticityToken, _) {
  return /* record */[
          /* debugName */component[/* debugName */0],
          /* reactClassInternal */component[/* reactClassInternal */1],
          /* handedOffState */component[/* handedOffState */2],
          /* willReceiveProps */component[/* willReceiveProps */3],
          /* didMount */component[/* didMount */4],
          /* didUpdate */(function (param) {
              console.log("feedbackhtml: " + param[/* newSelf */1][/* state */1][/* feedbackHTML */1]);
              return /* () */0;
            }),
          /* willUnmount */component[/* willUnmount */6],
          /* willUpdate */component[/* willUpdate */7],
          /* shouldUpdate */component[/* shouldUpdate */8],
          /* render */(function (param) {
              var send = param[/* send */3];
              var state = param[/* state */1];
              var updateFeedbackCB = function (param) {
                return Curry._1(send, /* UpdateFeedback */[param]);
              };
              return React.createElement("div", {
                          className: "feedback-form__container mt-2"
                        }, state[/* showForm */0] ? React.createElement("div", {
                                className: "feedback-form__trix-container"
                              }, ReasonReact.element(/* None */0, /* None */0, make(updateFeedbackCB, /* array */[])), React.createElement("button", {
                                    className: "btn btn-primary mt-1 mr-1",
                                    onClick: (function (param) {
                                        return sendFeedback(state, send, timelineEvent, authenticityToken, param);
                                      })
                                  }, React.createElement("i", {
                                        className: "fa fa-envelope mr-1"
                                      }), "Send"), React.createElement("button", {
                                    className: "btn btn-primary mt-1",
                                    onClick: (function () {
                                        Curry._1(send, /* UpdateFeedback */[""]);
                                        return Curry._1(send, /* ToggleForm */0);
                                      })
                                  }, "Cancel")) : React.createElement("button", {
                                className: "btn btn-primary mt-1",
                                onClick: (function () {
                                    return Curry._1(send, /* ToggleForm */0);
                                  })
                              }, React.createElement("i", {
                                    className: "fa fa-envelope mr-1"
                                  }), "Email Feedback"));
            }),
          /* initialState */(function () {
              return /* record */[
                      /* showForm */false,
                      /* feedbackHTML */""
                    ];
            }),
          /* retainedProps */component[/* retainedProps */11],
          /* reducer */(function (action, state) {
              if (action) {
                return /* Update */Block.__(0, [/* record */[
                            /* showForm */state[/* showForm */0],
                            /* feedbackHTML */action[0]
                          ]]);
              } else {
                return /* Update */Block.__(0, [/* record */[
                            /* showForm */!state[/* showForm */0],
                            /* feedbackHTML */state[/* feedbackHTML */1]
                          ]]);
              }
            }),
          /* subscriptions */component[/* subscriptions */13],
          /* jsElementWrapped */component[/* jsElementWrapped */14]
        ];
}

exports.UnexpectedResponse = UnexpectedResponse;
exports.handleApiError = handleApiError;
exports.TrixEditor = TrixEditor$1;
exports.str = str;
exports.toggleForm = toggleForm;
exports.updateFeedback = updateFeedback;
exports.clearFeedback = clearFeedback;
exports.handleResponseJSON = handleResponseJSON;
exports.sendFeedback = sendFeedback;
exports.component = component;
exports.make = make$1;
/* component Not a pure module */