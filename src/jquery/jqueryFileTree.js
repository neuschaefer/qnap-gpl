// jQuery File Tree Plugin
//
// Version 1.01
//
// Cory S.N. LaViska
// A Beautiful Site (http://abeautifulsite.net/)
// 24 March 2008
//
// Visit http://abeautifulsite.net/notebook.php?article=58 for more information
//
// Usage: $('.fileTreeDemo').fileTree( options, callback )
//
// Options:  root           - root folder to display; default = /
//           script         - location of the serverside AJAX file to use; default = jqueryFileTree.php
//           folderEvent    - event to trigger expand/collapse; default = click
//           expandSpeed    - default = 500 (ms); use -1 for no animation
//           collapseSpeed  - default = 500 (ms); use -1 for no animation
//           expandEasing   - easing function to use on expand (optional)
//           collapseEasing - easing function to use on collapse (optional)
//           multiFolder    - whether or not to limit the browser to one subfolder at a time
//           loadMessage    - Message to display while initial tree loads (can be HTML)
//
// History:
//
// 1.01 - updated to work with foreign characters in directory/file names (12 April 2008)
// 1.00 - released (24 March 2008)
//
// TERMS OF USE
// 
// jQuery File Tree is licensed under a Creative Commons License and is copyrighted (C)2008 by Cory S.N. LaViska.
// For details, visit http://creativecommons.org/licenses/by/3.0/us/
//
if(jQuery) (function($){
	
	$.extend($.fn, {
		fileTree: function(o, h, q, chgObj) {
			// Defaults
			if( !o ) var o = {};
			if( o.root == undefined ) o.root = '/';
			if( o.script == undefined ) o.script = 'jqueryFileTree.php';
			if( o.folderEvent == undefined ) o.folderEvent = 'click';
			if( o.expandSpeed == undefined ) o.expandSpeed= 500;
			if( o.collapseSpeed == undefined ) o.collapseSpeed= 500;
			if( o.expandEasing == undefined ) o.expandEasing = null;
			if( o.expandStart == undefined ) o.expandStart = null;				
			if( o.expandComplete == undefined ) o.expandComplete = null;		
			if( o.collapseEasing == undefined ) o.collapseEasing = null;
			if( o.collapseStart == undefined ) o.collapseStart = null;				
			if( o.collapseComplete == undefined ) o.collapseComplete = null;				
			if( o.multiFolder == undefined ) o.multiFolder = true;			
			if( o.loadMessage == undefined ) o.loadMessage = 'Loading...';
			if( o.el == undefined ) o.el = '';
			if( o.callbackInFile == undefined) o.callbackInFile = true;
			if( o.isAsync == undefined) o.isAsync = true;
			if( o.hilightRoot == undefined) o.hilightRoot = null;			
			$(this).each( function() {
				var timer1=0;
				var timer2=0;
				function showTree(c, t) {
					if($.isFunction(o.expandStart)) o.expandStart(c);
					$(c).addClass('wait');
					$(".jqueryFileTree.start").remove();
					//$.post(o.script + '&dir=' + encodeURIComponent(t), function(data) {						
					$.ajax({ 
						url: o.script + '&dir=' + encodeURIComponent(t), 
						async: o.isAsync,
						type: 'POST',
						success: function(data) {
							if(!Ext.isEmpty(chgObj)) data=data.replace(chgObj.regex,chgObj.change);
							
							if(t =="/")
							{
								$(c).html('');
								
							}
							else
							{
								$(c).find('.start').html('');
							}
							$(c).removeClass('wait').append(data);
							if( o.root == t )
								$(c).find('UL:hidden').show();
							else
								$(c).find('UL:hidden').slideDown({ duration: o.expandSpeed, easing: o.expandEasing});
							if($.isFunction(o.expandComplete)) o.expandComplete(c);
							bindTree(c);
						}
					});
				}
					
				function bindTree(t) {
					$(t).find('LI A').bind('dblclick', function() {
						var obj=$('#'+o.el);
						if(obj)
							obj.hide();
						if(q){
							q($(this).attr('rel'));	
						}
						obj.blur();
					//	alert($(this).attr('rel') + ' [' + $(this).attr('id')+']' );
						
						if(timer1!=0){
							clearTimeout(timer1);
							timer1=0;
						}
						if(timer2!=0){
							clearTimeout(timer2);
							timer2=0;	
						}
						return false;				
					});
					$(t).find('LI A').bind(o.folderEvent, function() {
						var self=this;
						if(o.hilightRoot){
							$(o.hilightRoot).find('LI A.hilight').removeClass('hilight');
							$j(self).addClass('hilight');
						}
						timer0 = setTimeout(function(){
							if( $(self).parent().hasClass('directory') ) {
								if (o.callbackInFile == false)
									h($(self).attr('rel'), {selectedNode:self});
								var restricted = false;
								if (o.checkFlag && $(self).data('flag'))
								{
									restricted = true;
									$(self).removeData('flag');
								}
								
								if (restricted == false)
								{									
									if( $(self).parent().hasClass('collapsed') ) {
										// Expand
										if( !o.multiFolder ) {
											$(self).parent().parent().find('UL').slideUp({ duration: o.collapseSpeed, easing: o.collapseEasing });
											$(self).parent().parent().find('LI.directory').removeClass('expanded').addClass('collapsed');
										}
										$(self).parent().find('UL').remove(); // cleanup
										showTree( $(self).parent(), $(self).attr('rel')/*.match( /.*\// )*/ );
										$(self).parent().removeClass('collapsed').addClass('expanded');
									} else {
										// Collapse
										if($.isFunction(o.collapseStart)) o.collapseStart($(self).parent());										
										$(self).parent().find('UL').slideUp({ duration: o.collapseSpeed, easing: o.collapseEasing });
										$(self).parent().removeClass('expanded').addClass('collapsed');
										if($.isFunction(o.collapseComplete)) o.collapseComplete($(self).parent());
									}									
								}
							} else {
								h($(self).attr('rel'));
							}
							return false;
						}, 200);
						if(timer1==0)
							timer1=timer0;
						else if(timer2==0)
							timer2=timer0;
					});
					
					// Prevent A from triggering the # on non-click events
					if( o.folderEvent.toLowerCase != 'click' ) $(t).find('LI A').bind('click', function() { return false; });
				}
				// Loading message
				$(this).html('<ul class="jqueryFileTree start"><li class="wait">' + o.loadMessage + '<li></ul>');
				// Get the initial file list
				showTree( $(this), o.root );
			});
		}
	});
	
})(jQuery);
