/**
  * Dropdown
  *
  * @description
  *
  * @param  config         An object of configuration settings:
  *
  *
  * @return new instance of Dropdown
  */





// Config defaults and init
// ----------------------------------------------------------------------------

var Dropdown = function (config) {
  this.hook = config.hook || 'js-drop';
  this.menu = config.menu;
  this.event = config.event || 'click';
  this.pane = document.getElementById(this.menu);
}


Dropdown.prototype.init = function() {
  this.modifyHooks(this.hook, this.addListener.bind(this, this.event));
}





// Shared methods
// ----------------------------------------------------------------------------

// grab element
Dropdown.prototype.modifyHooks = function(hook, func) {
  var elem = document.getElementById(hook); 
  // this.addBgListener(elem);
  func(elem);
}

// attach listeners to the document and menu items
Dropdown.prototype.addListener = function(event, elem) {
  document.addEventListener("mouseover", function(e) {
    if (e.target.closest("#"+this.hook)) {
      this.toggleMenu(elem);
    }
    else if (e.target.closest("#"+this.menu)) {return;}
    else if (this.pane.classList.contains('js-dropdown-visible')) {
      this.toggleMenu(elem);
    }   
  }.bind(this), false);
  document.addEventListener("mouseout", function(e) {
    if (e.target.closest("#"+this.hook)) {
      this.toggleMenu(elem);
    }
    else if (e.target.closest("#"+this.menu)) {return;}
    else if (this.pane.classList.contains('js-dropdown-visible')) {
      this.toggleMenu(elem);
    }   
  }.bind(this), false);
}

// toggle menu pane visibility
Dropdown.prototype.toggleMenu = function(elem) {
  if (this.pane.classList.contains('js-dropdown-hidden')) {
    this.pane.classList.replace('js-dropdown-hidden', 'js-dropdown-visible');
  } else if (this.pane.classList.contains('js-dropdown-visible')) {
    this.pane.classList.replace('js-dropdown-visible', 'js-dropdown-hidden');
  } 
}




// Include the dropdowns
// ----------------------------------------------------------------------------

var dropdowns = document.getElementsByClassName('js-drop');

if (dropdowns.length > 0) {
	var menus = [];

	Array.prototype.forEach.call(dropdowns, function(el) {
		menus.push(new Dropdown({'hook': el.id, 'menu': el.dataset.menu}));	
	});

	Array.prototype.forEach.call(menus, function(m) {
		m.init();
	}); 
}




// // Include the mobile dropdowns (unlike above, just writing it all here)
// // ----------------------------------------------------------------------------


// // open/close the big pane
// var mobileOpen  = document.getElementById('js-mobileOpen');
// var mobileClose = document.getElementById('js-mobileClose');
// var mobileMenu  = document.getElementById('js-mobileMenu');

// mobileOpen.addEventListener("click", function(e) {
//   mobileMenu .classList.replace("menu--closed", "menu--open");
// });

// mobileClose.addEventListener("click", function(e) {
//   mobileMenu .classList.replace("menu--open", "menu--closed");
// });


// // open/close individual menus

// var drawerHeads = document.getElementsByClassName('drawer-head'); 

// Array.prototype.forEach.call(drawerHeads, function(head){
  
//   head.addEventListener("click", function(e){
//     var mobmenu = head.dataset.mobmenu;
//     mobmenu = document.getElementById(mobmenu);
//     var arrow = this.querySelector(".drawer-arrow");

//     if (mobmenu.classList.contains('menu--closed')) {
//       mobmenu.classList.remove('menu--closed');
//       arrow.innerHTML = "&#9650;"
//     }
//     else {
//       mobmenu.classList.add('menu--closed');
//       arrow.innerHTML = "&#9660;"
//     }
    
//   });
// });

// // add listener to each of the links that toggles menus





