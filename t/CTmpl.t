# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl CTmpl.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';
my $MaxCount = 10000; # chang bencmark loops

#use Data::Dumper;
use Benchmark;
use Time::HiRes qw (time);
use Test::More tests => 5;
BEGIN { use_ok('CTmpl') };

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

my $CTmpl = new CTmpl;
is($CTmpl->version(), 'CTmpl version 2.0', 'Wrong version returned!');

my %tmpls = (
  'template' => 't/template.html',
  'included' => 't/included.html',
);
is($CTmpl->add_templates(\%tmpls), undef, "Template compiled with errors!");

#########################
my %Prms = (
  'main_title' => "Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title Long title",
  'main_body' => scalar LoadFile("Makefile"),
  'loop' => [ { a=>1 }, { b=>2 } ],
  'last_loop' => [ { vlp => 1, value => 1}, { vlp => 2, value => 2},{ vlp => 3, value => 3},{ vlp => 4, value => 4},{ vlp => 5, value => 5}, { vlp => 6, value => 6} ],
  'flop2' => [ { vlp => 1, value => 1}, { vlp => 2, value => 2},{ vlp => 3, value => 3},{ vlp => 4, value => 4},{ vlp => 5, value => 5}, { vlp => 6, value => 6} ],
  'enm1' => 10,
  'enm2' => 1,
  'enm3' => 3,
  'enm4' => 4,
  'enm5' => 5,
  'enm6' => 6
);
for(my $i=0;$i<6; $i++) {
	$Prms{'last_loop'}[$i]{'last_loop'} = [ { vlp => 1, value => 1}, { vlp => 2, value => 2},{ vlp => 3, value => 3},{ vlp => 4, value => 4},{ vlp => 5, value => 5}, { vlp => 6, value => 6} ];
}
my $test = length($CTmpl->gen_template("template", \%Prms));
is(($test>34000&&$test<36000)?1:0, 1, "Template generated with errors, something wrong!");
is(length($CTmpl->get_compiled_struct("template")), 25318, "Template structure generated with errors, something wrong!");

my $t0 = new Benchmark;
my $ut0 = time();
my $ass;
for(my $i=0; $i<$MaxCount; $i++) {
  $ass = $CTmpl->gen_template("template", \%Prms);
}
my $t1 = new Benchmark;
my $ut1 = time();

my $utl = $ut1 - $ut0;
diag("Benchmark template len: ".(-s $tmpls{'template'})." last size: ".length($ass)." x $MaxCount. $utl sec (".($utl/$MaxCount).") ".timestr(timediff($t1, $t0),"all","5.5f"));

sub LoadFile {
	local(*IN);
	open (IN, $_[0]);
	my @lines = <IN>;
	close (IN);
	unless (wantarray) {
		return join('',@lines);
	} else {
		return @lines;
	};
} 

