#!/usr/bin/perl

# The tool will convert the templates of HTML::Template

use POSIX qw(strftime);
use Data::Dumper;
use Cwd;

  $::SpecFiles = "";
  $::Extensions = "html";

  open (LOG, "+<convert.log") || open (LOG, ">convert.log") || die "$!";
  seek LOG, 0, 2;
  
  my $time = strftime("%a %b %e %H:%M:%S %Y", gmtime());
  print LOG "Started at $time\n--------------------------------------------------\n";
  
  my @Files;
  getDirContent(getcwd, \@Files);
  
  my $Bad = 0;
  my $Good = 0;
  
  my $TEMP;
  my %Stats;
  foreach my $file (@Files) {
    open (FL, "+<$file") || die "$!";
    binmode FL;
    seek (FL, 0, 0);
    my $Text;
    read (FL, $Text, (stat FL)[7]);
    my $modif = 0;
    while ( $Text =~ m/(<TMPL.*?>)/ig ) { $Stats{$1} += 1; }
    print LOG "Converting file '".$file."'\n";
    $file =~ s/^(.*?)\.html$/$1\.tmpl/i;
    $file =~ m/blank\/(.*?)[\/]?([^\/]*?)\.tmpl$/i;
    $TEMP .= "'".(($1)?"$1_$2":$2)."' => '$file',\n";
    
    open (FOUT, ">$file") || die "$!";
    print FOUT ChangeTemplate($Text);
    close FOUT;
  }
  print LOG Dumper(\%Stats);
  $time = strftime("%a %b %e %H:%M:%S %Y", gmtime());
  print LOG "--------------------------------------------------\nEnded at $time\nFiles parsed ".($Bad+$Good).", $Bad incorrect and $Good correct...\n\n";

close LOG;

    open (FOUT, ">convert.lst") || die "$!";
    print FOUT $TEMP;
    close FOUT;

    sub getDirContent {
  my ($dir, $ref1) = @_;

  opendir (DIR, $dir) || die "$!";
	my @Objects = grep !/^\.\.?$/, readdir DIR;
	closedir DIR;

  foreach my $name (@Objects) {
    my $fnm = $dir."/".$name;
    if (-d $fnm) {
      getDirContent($fnm, $ref1);
    } else {
      push @{$ref1}, $fnm if ($name =~ /^($::SpecFiles)$/ || $name =~ /\.($::Extensions)$/); 
    }
  }
}


sub ChangeTemplate {
  my $src = shift;
  my $dst = '';
  my $rec = 0;
  $src =~ s/#/__HASH_TAG__/g;
  while (length $src > 0 ) {
    $cur = index ($src, '<');
    if ($cur >= 0) {
      $dst .= substr $src, 0, $cur, "";
      my $i2 = index($src,'>',1);
      my $i3 = index($src,'<',1);
      if ($i3>=0 && $i2 >=0 && $i3 < $i2) { 
#        print LOG "!!!WARN!!! $i3 < $i2 ".(substr $src, 0, $i3);
        $dst .= substr $src, 0, $i3, "";
#        print LOG "\nnext ('".(substr $src, 0, 30)."')\n";
        redo;
      }
      #print LOG "ALLS tag: ".(substr $src, 0, $i2+1)."\n";
      if ($i2 > 0 ) {
        my $tag = substr $src, 1, ($i2-1);
        if ($tag =~ m/^(\/?TMPL_.*?)( NAME=['"](.*?)['"]( ESCAPE='(.*?)')?)?$/) {
          my $s2 = $1;
          my $s4 = $3;
          print LOG "tag: ".(substr $src, 0, $i2+1)."\n";
          if ($s2 =~ m/^TMPL_VAR$/) {
            $dst .= "##$s4##";
            substr ($src, 0, $i2+1, "");
          } elsif ($s2 =~ m/^TMPL_INCLUDE$/) {
            $s4 =~ m@\/([^\/]*?)(\.[^\.]*)?$@;
            $dst .= "#^$1#:$1#<!-- $s4 -->\$";
            substr ($src, 0, $i2+1, "");
          } elsif ($s2 =~ m/^TMPL_IF$/) {
            $dst .= "#!$s4#?";
            substr ($src, 0, $i2+1, "");
            gen_if_then_else(\$dst, \$src, \$rec);
          } elsif ($s2 =~ m/^TMPL_UNLESS$/) {
            $dst .= "#!!$s4#?";
            substr ($src, 0, $i2+1, "");
            gen_unless_then_else(\$dst, \$src, \$rec);
          } elsif ($s2 =~ m/^TMPL_LOOP$/) {
            $dst .= "#\@$s4#:";
            substr ($src, 0, $i2+1, "");
            gen_loop(\$dst, \$src, \$rec);
          } else {
            print LOG "$s2 è $s4  0 - $i2".(substr $src, 0, $i2+1)."\n";
            $dst .= substr ($src, 0, $i2+1, "");
          }
        } else {
          $dst .= substr ($src, 0, $i2+1, "");
        }
      } else {
        $dst .= $src;
        $src = "";
        last;
      }
    } else {
      $dst .= $src;
      $src = "";
      last;
    }
  }
  $dst =~ s/__HASH_TAG__#/#|#/g;
  $dst =~ s/__HASH_TAG__/#/g;
  return $dst;
}

sub gen_draw_block {
  my ($dst, $src, $rec, @end ) = (  $_[0] , $_[1], $_[2], @{ $_[3] } );
  $$rec++;
  while (length $$src > 0 ) {
    $cur = index ($$src, '<');
    if ($cur >= 0) {
      $$dst .= substr $$src, 0, $cur, "";
      my $i2 = index($$src,'>',1);
      my $i3 = index($$src,'<',1);
      if ($i3>=0 && $i2 >=0 && $i3 < $i2) {
#        print LOG "!!!WARN!!! $i3 < $i2 ".(substr $$src, 0, $i3);
        $$dst .= substr $$src, 0, $i3, "";
#        print LOG "\nnext ('".(substr $$src, 0, 30)."')\n";
        redo;
      }
      if ($i2 > 0 ) {
        my $tag = substr $$src, 1, ($i2-1);
        if ($tag =~ m/^(\/?TMPL_.*?)( NAME=['"](.*?)['"]( ESCAPE='(.*?)')?)?$/) {
          my $s2 = $1;
          my $s4 = $3;
          print LOG sprintf("%*.*s", $$rec*2, $$rec*2, '                                                  ')." [$$rec] gen_draw_block  '$s2' name=$s4\n";
          for (my $i=0; $i <= $#end; $i++) {
            print LOG sprintf("%*.*s", $$rec*2, $$rec*2, '                                                  ')."* $i] $s2 eq $end[$i]\n";
            if ($s2 eq $end[$i]) {
              substr ($$src, 0, $i2+1, "");
              $$rec--;
              return $i;
            }
          }
          if ($s2 =~ m/^TMPL_VAR$/) {
            $$dst .= "##$s4##";
            substr ($$src, 0, $i2+1, "");
          } elsif ($s2 =~ m/^TMPL_INCLUDE$/) {
            $s4 =~ m@\/([^\/]*?)(\.[^\.]*)?$@;
            $dst .= "#^$1#:$1#<!-- $s4 -->\$";
            substr ($$src, 0, $i2+1, "");
          } elsif ($s2 =~ m/^TMPL_IF$/) {
            $$dst .= "#!$s4#?";
            substr ($$src, 0, $i2+1, "");
            gen_if_then_else($dst, $src, $rec);
          } elsif ($s2 =~ m/^TMPL_UNLESS$/) {
            $$dst .= "#!!$s4#?";
            substr ($$src, 0, $i2+1, "");
            gen_unless_then_else($dst, $src, $rec);
          } elsif ($s2 =~ m/^TMPL_LOOP$/) {
            $$dst .= "#\@$s4#:";
            substr ($$src, 0, $i2+1, "");
            gen_loop($dst, $src, $rec);
          } else {
            print LOG sprintf("%*.*s", $$rec*2, $$rec*2, '                                                  ')."* gen_draw_block $s2 è $s4  0 - $i2".(substr $$src, 0, $i2+1)."\n";
            $$dst .= substr ($$src, 0, $i2+1, "");
          }
        } else {
          $$dst .= substr ($$src, 0, $i2+1, "");
        }
  
      } else {
        $$dst .= $$src;
        $$src = "";
        last;
      }
    } else {
      $$dst .= $$src;
      $$src = "";
      last;
    }
  }
  $$rec--;
  return -1;
}  


sub gen_if_then_else {
  my ($dst, $src, $rec) = @_;
  $$rec++;
  my $else = 0;
  my $i = 0;
  do {
    $i = gen_draw_block($dst, $src,  $rec, [ '/TMPL_IF', 'TMPL_ELSE' ]);
    if ($i==1) {
      $else++;
      $$dst .= '#:';
    } else { 
      $$dst .= ($else)?'#$':'#:#$';
    }
  } while ($i > 0);
  $$rec--;
}

sub gen_unless_then_else {
  my ($dst, $src, $rec) = @_;
  $$rec++;
  my $else = 0;
  my $i = 0;
  do {
    $i = gen_draw_block($dst, $src,  $rec, [ '/TMPL_UNLESS', 'TMPL_ELSE' ]);
    if ($i==1) {
      $else++;
      $$dst .= '#:';
    } else { $$dst .= ($else)?'#$':'#:#$'; }
  } while ($i > 0);
  $$rec--;
}

sub gen_loop{
  my ($dst, $src, $rec) = @_;
  $$rec++;
  gen_draw_block($dst, $src,  $rec, [ '/TMPL_LOOP' ]);
  $$dst .= '#$';
  $$rec--;
}
















