/*
 * XKeyboard.i
 * SWiG interface file for XKB ruby extension class
 * Copyright (C) 2008 by Jay Bromley <jbromley@gmail.com>
 * $Id$
 */

%module xkb
%{
#include "/usr/lib/ruby/1.8/i686-linux/ruby.h"
#include "XKeyboard.h"
%}

/* Include C++ standard library stuff. */
%include std_vector.i
%include std_string.i
%exceptionclass X11Exception;

/* 
 * Rename methods to be more Ruby-like. Next time, I have to use the Ruby
 * naming scheme directly in the C/C++ code to avoid this.
 */
%rename("group_count") XKeyboard::groupCount() const;
%rename("group_names") XKeyboard::groupNames() const;
%rename("group_symbols") XKeyboard::groupSymbols() const;
%rename("current_group_num") XKeyboard::currentGroupNum() const;
%rename("current_group_name") XKeyboard::currentGroupName() const;
%rename("current_group_symbol") XKeyboard::currentGroupSymbol() const;
%rename("set_group_by_num") XKeyboard::setGroupByNum(int groupNum);
%rename("change_group") XKeyboard::changeGroup(int increment);

/* Map type std::vector<std::string> to a Ruby array of strings. */
%typemap(out) std::vector<std::string> {
    $result = rb_ary_new2($1.size());
    for (unsigned int i = 0; i < $1.size(); i++) {
	std::string str = $1.at(i);
	rb_ary_store($result, i, rb_str_new(str.c_str(), str.size()));
    }
}

/* Main C++ header */
%include XKeyboard.h

/*
Local Variables:
mode: c++
End:
*/
