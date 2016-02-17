# Generate make file for XKB interface Ruby extension
# $Id: extconf.rb 26 2008-04-09 08:47:11Z jay $

require 'mkmf'
$libs = append_library($libs, "X11")
$libs = append_library($libs, "stdc++")
create_makefile('ruby-wmii/xkb');
