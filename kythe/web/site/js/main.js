/*
 * Copyright 2015 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Modify each output listingblock (generated from Asciidoc) to be in a
// collapsible Bootstrap panel.
$(document).ready(function() {
  $('.listingblock.output')
      .addClass('panel panel-default')
      .removeClass('listingblock output')
      .each(function(_, el) {
    var id = $(el).find('.content')
        .addClass('collapse in panel-body')
        .uniqueId().attr('id');
    $(el).find('.title')
        .replaceWith('<div class="panel-heading"><a class="title" data-toggle="collapse" href="#' + id + '">Output</a></div>');
  });
});
