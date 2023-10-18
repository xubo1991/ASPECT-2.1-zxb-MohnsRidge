/*
  Copyright (C) 2018 by the authors of the ASPECT code.

  This file is part of ASPECT.

  ASPECT is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  ASPECT is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ASPECT; see the file LICENSE.  If not see
  <http://www.gnu.org/licenses/>.
*/


demangleStrings();
reorderList();

function demangleStrings() {
  mangledStrings = document.getElementsByClassName("mangled");
  var j;
  for (j = 0; j < mangledStrings.length; j++) {
    mangledStrings[j].innerHTML = mangledStrings[j].innerHTML.replace(/_20/g," ");
    mangledStrings[j].innerHTML = mangledStrings[j].innerHTML.replace(/_2d/g,"-");
  }
  return;
}

function expand(collection) {
  var i;
  for (i = 0; i < collection.length; i++) {
    collection[i].classList.add("active");
    var content = collection[i].nextElementSibling;
    content.style.display = "block";
  }
}

function collapse(collection) {
  var i;
  for (i = 0; i < collection.length; i++) {
    collection[i].classList.remove("active");
    var content = collection[i].nextElementSibling;
    content.style.display = "none";
  }
}

function expandAll() {
  var coll = document.getElementsByClassName("collapsible");
  expand(coll)
}

function collapseAll() {
  var coll = document.getElementsByClassName("collapsible");
  collapse(coll)
}

function expandAllSubsections() {
  var coll = document.getElementsByClassName("subsection");
  expand(coll)
}

function collapseAllSubsections() {
  var coll = document.getElementsByClassName("subsection");
  collapse(coll)
}

var coll = document.getElementsByClassName("collapsible");
var i;

for (i = 0; i < coll.length; i++) {
  coll[i].addEventListener("click", function() {
    this.classList.toggle("active");
    var content = this.nextElementSibling;
    if (content.style.display === "block") {
      content.style.display = "none";
    } else {
      content.style.display = "block";
    }
  });
}

function sortTopNodes(ClassType) {
  var list, i, switching, shouldSwitch;
  list = document.getElementById("ParameterList");
  switching = true;

  /* Make a loop that will continue until
  no switching has been done: */
  while (switching) {
    // Start by saying: no switching is done:
    switching = false;
    children = list.children;
    // parameters = children.getElementbyClassName("parameter");

    // Loop through all list items:
    for (i = 0; i < (children.length - 1); i++) {
      if (children[i].children[0].classList.contains(ClassType) && children[i+1].children[0].classList.contains(ClassType)) {
        // Start by saying there should be no switching:
        shouldSwitch = false;
        /* Check if the next item should
        switch place with the current item: */
        if (children[i].children[0].innerHTML.toLowerCase() > children[i + 1].children[0].innerHTML.toLowerCase()) {
          /* If next item is alphabetically lower than current item,
          mark as a switch and break the loop: */
          shouldSwitch = true;
          break;
        }
      }
    }

    if (shouldSwitch) {
      /* If a switch has been marked, make the switch
      and mark the switch as done: */
      children[i].parentNode.insertBefore(children[i + 1], children[i]);
      switching = true;
    }
  }
}

function reorderList() {
  var list, i;
  list = document.getElementById("ParameterList");

  /* Move parameters to the front */
  children = list.children;

  for (i = 0; i < children.length; i++) {
    if (children[i].children[0].classList.contains("parameter")) {
	  children[i].parentNode.insertBefore(children[i], children[0]);
    }
  }

  sortTopNodes("parameter");
  sortTopNodes("subsection");
}
