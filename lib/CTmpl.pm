package CTmpl;

use strict;
use vars qw($VERSION @ISA);

our $VERSION = '2.00';

require XSLoader;
XSLoader::load('CTmpl', $VERSION);

# Preloaded methods go here.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

CTmpl - Perl extension for fast generation templates.

=head1 SYNOPSIS

  use CTmpl;
  
  my $Tmpl = new CTmpl({'register' => 'templates/register.tpl'});
  my %prms = ( 'param_name' => "Here value" );
  print $Tmpl->gen_template('register', \%prms );
  
=head1 DESCRIPTION

 #          - #| slashed #. example : bgcolor="###color##"  - error
                                    bgcolor="#|##color##" - normal
 var   		  - ##VAR##
 if    		  - #!BOOL#? if true #: if false #$
 unless		  - #!!BOOL#? if false #: if true #$
 enum  		  - #%ENUM#? index 0 #: index 1 #: index 2 #: index N and etc #: index undef or last #$
 loop  		  - #@LOOP#: any text or block here #$
    in a loop you can use these markers
    __first__     - true if draw first object in loop
    __last__      - true if draw last object in loop
    __even__      - true if draw even object in loop
    __odd__       - true if draw odd object in loop
  include   - #^HASH#:TEMPLATE_NAME#$

=head1 TODO

  maybe some optimization

=head1 EXAMPLES

  Template:
    #!value#?true#:false#$
  Value     - Result
    0       - false
    1       - true
   "str"    - true
   @arr     - if length > 0 then true, else false
   %hash    - if number of keys > 0 then true, else false
  reference - dereference first then check as described above
  
  Templates:
    register:
      #@loop#:
        #^header#?header#$<br>
      #$
    header:
      ##some_variable##

    my $Tmpl = new CTmpl();
    $Tmpl->add_templates(
      {'register' => 'templates/register.tpl'}
      {'header' => 'templates/header.tpl'}
    );
    my %prms = ( 'loop' => [
      { 'header' => { some_variable => 'variable 1' } },  # Included
      { 'header' => { },                                  # It would not be included because the hash is 0-key
      { 'header' => { some_variable => 'variable 3' } },  # Included
    );
    print $Tmpl->gen_template('register', \%prms );
  In browser you will see next:
    variable 1
    variable 3
  
    
   
=head2 EXPORT

None by default.

=head1 AUTHOR

E. V. Stolyarenko, E<lt>devouer@yandex.ruE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2007 by E. V. Stolyarenko

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.4 or,
at your option, any later version of Perl 5 you may have available.


=cut
