#use 5.008004;
use ExtUtils::MakeMaker;
use Config;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
#$Verbose = 1;
my $Fast = 0;
my $UseTCMalloc = 0;
while ($_ = $ARGV[0], /^-/) {
  shift;
  last if /^--$/;
  if (/^-f/) { $Fast = 1 }
  elsif (/^-t/) { $UseTCMalloc = 1 }
  elsif (/^-v/) { $Verbose++  }
}
open (fh, ">config.inc");
print fh "\$Fast = $Fast;\n";
print fh "\$UseTCMalloc = $UseTCMalloc;\n";
print fh "\$Verbose = $Verbose;\n1\n";
close fh;
$CC = 'g++';
my $CCFLAGS = $Config{'ccflags'};
# Perl is built with -Wdeclaration-after-statement on RHEL5 - this isn't
# meaningful for C++ - it only emits a warning but it's easy to fix.
$CCFLAGS =~ s/(?:^|\s+)-Wdeclaration-after-statement(?:\s+|$)/ /;
$CCFLAGS .= " -Wno-deprecated";
WriteMakefile(
    NAME              => 'CTmpl',
    VERSION_FROM      => 'lib/CTmpl.pm', # finds $VERSION
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/CTmpl.pm', # retrieve abstract from module
       AUTHOR         => 'E. V. Stolyarenko <devouer@yandex.ru>') : ()),
    LIBS              => [ ($UseTCMalloc) ? '-ltcmalloc' : '' ], # e.g., '-lm'
    DEFINE            => ($Fast)?'-DFAST=1':'', # e.g., '-DHAVE_SOMETHING'
    INC               => '-I. -I/usr/include', # e.g., '-I. -I/usr/include/other'
    MYEXTLIB          => 'templates/libtemplates$(LIB_EXT)',
    CC                => $CC,
    CCFLAGS           => $CCFLAGS,
    LD                => '$(CC)',
    XSOPT             => '-C++ -hiertype',
    XSPROTOARG        => '',
    TYPEMAPS          => ['perlobject.map'],
);
sub MY::postamble {
'
$(MYEXTLIB): templates/Makefile
	cd templates && $(MAKE) $(PASSTHRU)
';
}
