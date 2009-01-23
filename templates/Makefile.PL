use ExtUtils::MakeMaker;
use Config;
require "../config.inc" if -e "../config.inc";
$CC = 'gcc';
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
my $CCFLAGS = $Config{'ccflags'};
# Perl is built with -Wdeclaration-after-statement on RHEL5 - this isn't
# meaningful for C++ - it only emits a warning but it's easy to fix.
$CCFLAGS =~ s/(?:^|\s+)-Wdeclaration-after-statement(?:\s+|$)/ /;
$CCFLAGS .= " -Wno-deprecated";
WriteMakefile(
    NAME        => 'CTmpl::templates',
    SKIP        => [qw(all static dynamic)],
    LIB         => ($UseTCMalloc) ? '-ltcmalloc' : '',
    INC         => '-I. -I/usr/include', # e.g., '-I. -I/usr/include/other
    DEFINE      => ($Fast)?'-DFAST=1':'', # e.g., '-DHAVE_SOMETHING'
    clean       => {'FILES' => 'libtemplates$(LIB_EXT)'},
    CC          => $CC,
    LD          => '$(CC)',
    CCFLAGS     => $CCFLAGS,
    XSOPT       => '-C++ -hiertype',
    XSPROTOARG  => '',    	
);

sub MY::top_targets {
'
all :: static
	
pure_all :: static
	
static :: libtemplates$(LIB_EXT)

libtemplates$(LIB_EXT): $(O_FILES)
	$(AR) cr libtemplates$(LIB_EXT) $(O_FILES)
	$(RANLIB) libtemplates$(LIB_EXT)
';
}
