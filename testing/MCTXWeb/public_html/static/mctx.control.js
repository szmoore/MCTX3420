/**
 * Code for the controls page.
 * @date 19-10-2013
 */

mctx.control = {};
mctx.control.api = mctx.api + 'control'

$(document).ready(function () {
  $.ajax({
    url : mctx.control.api,
    data : {'action' : 'identify'}
  }).done(function () {
    
  });
});

