/*
 * SimpleModal Basic Modal Dialog
 * http://www.ericmmartin.com/projects/simplemodal/
 * http://code.google.com/p/simplemodal/
 *
 * Copyright (c) 2008 Eric Martin - http://ericmmartin.com
 *
 * Licensed under the MIT license:
 *   http://www.opensource.org/licenses/mit-license.php
 *
 * Revision: $Id: basic.js,v 1.1.1.1 2009/02/11 08:51:56 ricky Exp $
 *
 */

$(document).ready(function () {
	$('#basicModal input:eq(0)').click(function (e) {
		e.preventDefault();
		$('#basicModalContent').modal();
	});
});