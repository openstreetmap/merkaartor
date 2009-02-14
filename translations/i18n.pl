#! /usr/bin/perl -w

use utf8;
use encoding "utf8";
use Term::ReadKey;

my $nocontext = 1;
my $waswarn = 0;
my $mail = "Merkaartor <merkaartor\@openstreetmap.org>";
my %pokeys = (
);

# don't copy these from files
my %defkeys = (
  "X-Generator" => "Merkaartor translation convert",
  "MIME-Version" => "1.0",
  "Content-Type" => "text/plain; charset=UTF-8",
  "Content-Transfer-Encoding" => "8bit",
  "Project-Id-Version" => "merkaartor_templates 1.0",
  "Report-Msgid-Bugs-To" => $mail,
  "POT-Creation-Date" => getdate(),
  "PO-Revision-Date" => getdate(),
#  "Last-Translator" => $mail,
#  "Language-Team" => $mail,
#  "X-Launchpad-Export-Date" => getdate(),
);

main();

sub getdate
{
  my @t=gmtime();
  return sprintf("%04d-%02d-%02d %02d:%02d+0000",
  1900+$t[5],$t[4]+1,$t[3],$t[2],$t[1]);
}

sub loadfiles($$@)
{
  my $desc;
  my $all;
  my ($lang,$keys,@files) = @_;
  foreach my $file (@files)
  {
    die "Could not open file $file." if(!open FILE,"<:utf8",$file);
    my $linenum = 0;
    if($file =~ /\.mat$/)
    {
      while(my $line = <FILE>)
      {
        ++$linenum;
        chomp $line;
        if($line =~ /^ *<description +locale="([A-Za-z_]+)" *>(.*?)<\/description> *$/)
        {
          my $val = maketxt($2);
          my $l = $1;
          $desc{$l} = $val;
          $desc{_file} = "$file:$linenum" if($l eq "en");
        }
        elsif($line =~ /description/)
        {
          die "Can't handle line $linenum in $file: $line";
        }
        elsif(%desc)
        {
          my $en = $desc{"en"};
          die "No english string found in previous block line $linenum in $file: $line" if(!$en);
          delete $desc{"en"};
          foreach my $l (reverse sort keys %desc)
          {
            copystring(\%all, $en, $l, $desc{$l}, "line $linenum in $file", undef, 0);
            ++$lang->{$l} if !($l =~ /^_/);
          }
          %desc = ();
        }
      }
    }
    elsif($file =~ /[-_](.._..)\.po$/ || $file =~ /^(?:.*\/)?(.._..)\.po$/ ||
    $file =~ /[-_](..)\.po$/ || $file =~ /^(?:.*\/)?(..)\.po$/)
    {
      my $l = $1;
      ++$lang->{$l};
      my %postate = (last => "", type => "");
      my $linenum = 0;
      while(<FILE>)
      {
        ++$linenum;
        my $fn = "$file:$linenum";
        chomp;
        if($_ =~ /^#/ || !$_)
        {
          checkpo(\%postate, \%all, $l, "line $linenum in $file", $keys);
          $postate{fuzzy} = ($_ =~ /fuzzy/);
        }
        elsif($_ =~ /^"(.*)"$/) {$postate{last} .= $1;}
        elsif($_ =~ /^(msg.+) "(.*)"$/)
        {
          my ($n, $d) = ($1, $2);
          checkpo(\%postate, \%all, $l, "line $linenum in $file", $keys);
          $postate{last} = $d;
          $postate{type} = $n;
          $postate{src} = $fn if($n eq "msgid");
        }
        else
        {
          die "Strange line $linenum in $file: $_.";
        }
      }
      checkpo(\%postate, \%all, $l, "line $linenum in $file", $keys);
    }
    elsif($file =~ /\.ts$/)
    {
      my $linenum = 0;
      my $l;
      my $ctx;
      my $loc;
      my $issource;
      my $istrans;
      my $source;
      my @trans;
      my $fuzzy;
      my $numerus;
      while(<FILE>)
      {
        ++$linenum;
        if(/<name>(.*)<\/name>/) { $ctx = $1; }
        elsif(/<location filename="(.*?)" line="(.*?)"\/>/) { $loc = "$1:$2"; }
        elsif(/context>/){$ctx = undef;}
        elsif(/message( numerus="yes")?>/)
        {
          my $n = $1;
          if($source)
          {
            $source = maketxt($source);
            if(!$fuzzy)
            {
              my $txt = "line $linenum in $file";
              $txt .= ", $loc" if($loc);
              for($i = 0; $i <= $#trans; ++$i)
              {
                copystring(\%all, $source, $i ? "$l.$i" : $l, maketxt($trans[$i]), $txt, $ctx, 0);
              }
              if(defined($numerus))
              {
                copystring(\%all, $source, "en.1", $source, $txt, $ctx, 0);
              }
            }
            copystring(\%all, $source, "_file", $loc, $txt, $ctx, 0) if $loc;
            copystring(\%all, $source, "_src.$l", "$file:$linenum", $txt, $ctx, 0);
          }
          @trans = undef;
          $loc = $issource = $istrans = $source = $numerus = $fuzzy = undef;
          $numerus = 0 if $n;
        }
        elsif(/<TS .* language="(.*)">/) { $l = getlang($1); ++$lang->{$l}; }
        elsif(/<\?xml/ || /<!DOCTYPE/ || /<\/TS>/ || /<defaultcodec>/){} # ignore
        # source
        elsif(/<source>(.*)<\/source>/){$source = $1;}
        elsif(/<source>(.*)/){$source = "$1\n"; $issource = 1;}
        elsif($issource && /(.*)<\/source>/){$source .= $1; $issource = undef;}
        elsif($issource){$source .= $_;}
        # translation
        elsif(defined($numerus) && /translation(?: type="(unfinished|obsolete)")?>/) {$fuzzy=$1;}
        elsif(defined($numerus) && /<numerusform>(.*)<\/numerusform>/){$trans[$numerus++] = $1;}
        elsif(defined($numerus) && /<numerusform>(.*)/){$trans[$numerus] = "$1\n"; $istrans = 1;}
        elsif(defined($numerus) && $istrans && /(.*)<\/numerusform>/){$trans[$numerus++] .= $1; $istrans = undef;}
        elsif(/<translation(?: type="(unfinished|obsolete)")?>(.*)<\/translation>/){$trans[0] = $2;$fuzzy=$1;}
        elsif(/<translation(?: type="(unfinished|obsolete)")?>(.*)/){$trans[0] = "$2\n"; $istrans = 1;$fuzzy=$1;}
        elsif($istrans && /(.*)<\/translation>/){$trans[0] .= $1; $istrans = undef;}
        elsif($istrans){$trans[$numerus ? $numerus : 0] .= $_;}
        else
        {
          die "Strange line $linenum in $file: $_.";
        }

      }
    }
    else
    {
      die "File format not supported for file $file.";
    }
    close(FILE);
  }
  return %all;
}

my $alwayspo = 0;
my $alwaysup = 0;
my $noask = 0;
my $conflicts;
sub copystring($$$$$$$)
{
  my ($data, $en, $l, $str, $txt, $context, $ispo) = @_;

  $en = "___${context}___$en" if $context && !$nocontext;

  if(exists($data->{$en}{$l}) && $data->{$en}{$l} ne $str)
  {
    return if !$str;
    if($l =~ /^_/)
    {
      $data->{$en}{$l} .= ";$str" if !($data->{$en}{$l} =~ /$str/);
    }
    elsif(!$data->{$en}{$l})
    {
      $data->{$en}{$l} = $str;
    }
    else
    {

      my $f = $data->{$en}{_file} || "";
      $f = ($f ? "$f;".$data->{$en}{"_src.$l"} : $data->{$en}{"_src.$l"}) if $data->{$en}{"_src.$l"};
      my $isotherpo = ($f =~ /\.po\:/);
      my $pomode = ($ispo && !$isotherpo) || (!$ispo && $isotherpo);

      my $mis = "String mismatch for '$en' **$str** ($txt) != **$data->{$en}{$l}** ($f)\n";
      my $replace = 0;

      if(($conflicts{$l}{$str} || "") eq $data->{$en}{$l}) {}
      elsif($pomode && $alwaysup) { $replace=$isotherpo; }
      elsif($pomode && $alwayspo) { $replace=$ispo; }
      elsif($noask) { print $mis; ++$waswarn; }
      else
      {
        ReadMode 4; # Turn off controls keys
        my $arg = "(l)eft, (r)ight";
        $arg .= ", (p)o, (u)pstream[ts/mat], all p(o), all up(s)tream" if $pomode;
        $arg .= ", e(x)it: ";
        print "$mis$arg";
        while((my $c = getc()))
        {
          if($c eq "l") { $replace=1; }
          elsif($c eq "r") {}
          elsif($c eq "p" && $pomode) { $replace=$ispo; }
          elsif($c eq "u" && $pomode) { $replace=$isotherpo; }
          elsif($c eq "o" && $pomode) { $alwayspo = 1; $replace=$ispo; }
          elsif($c eq "s" && $pomode) { $alwaysup = 1; $replace=$isotherpo; }
          elsif($c eq "x") { $noask = 1; ++$waswarn; }
          else { print "\n$arg"; next; }
          last;
        }
        print("\n");
        ReadMode 0; # Turn on controls keys
      }
      if(!$noask)
      {
        if($replace)
        {
          $data->{$en}{$l} = $str;
          $conflicts{$l}{$data->{$en}{$l}} = $str;
        }
        else
        {
          $conflicts{$l}{$str} = $data->{$en}{$l};
        }
      }
    }
  }
  else
  {
    $data->{$en}{$l} = $str;
  }
}

sub checkpo($$$$$)
{
  my ($postate, $data, $l, $txt, $keys) = @_;
  if($postate->{type} eq "msgid") {$postate->{msgid} = $postate->{last};$postate->{msgid_pl}="";}
  elsif($postate->{type} eq "msgid_plural") {$postate->{msgid_1} = $postate->{last};}
  elsif($postate->{type} eq "msgstr[0]") {$postate->{msgstr} = $postate->{last};}
  elsif($postate->{type} eq "msgstr[1]") {$postate->{msgstr_1} = $postate->{last};}
  elsif($postate->{type} eq "msgstr[2]") {$postate->{msgstr_2} = $postate->{last};}
  elsif($postate->{type} eq "msgstr[3]") {$postate->{msgstr_3} = $postate->{last};}
  elsif($postate->{type} eq "msgctxt") {$postate->{context} = $postate->{last};}
  else
  {
    if($postate->{type} eq "msgstr") {$postate->{msgstr} = $postate->{last};}
    elsif($postate->{type}) { die "Strange type $postate->{type} found\n" }
    if((!$postate->{fuzzy}) && $postate->{msgstr} && $postate->{msgid})
    {
      copystring($data, $postate->{msgid}, $l, $postate->{msgstr},$txt,$postate->{context}, 1);
      if($postate->{msgstr_1})
      { copystring($data, $postate->{msgid}, "$l.1", $postate->{msgstr_1},$txt,$postate->{context}, 1); }
      if($postate->{msgid_1})
      { copystring($data, $postate->{msgid}, "en.1", $postate->{msgid_1},$txt,$postate->{context}, 1); }
      copystring($data, $postate->{msgid}, "_src.$l", $postate->{src},$txt,$postate->{context}, 1);
    }
    elsif($postate->{msgstr} && !$postate->{msgid})
    {
      my %k = ($postate->{msgstr} =~ /(.+?): +(.+?)\\n/g);
      # take the first one!
      for $a (sort keys %k)
      {
        $keys->{$l}{$a} = $k{$a} if !$keys->{$l}{$a};
      }
    }
    delete $postate->{msgid};
    delete $postate->{msgstr};
    delete $postate->{msgid_1};
    delete $postate->{msgstr_1};
    delete $postate->{context};
    $postate->{type} = "";
  }
}

sub createpos($$@)
{
  my ($data, $keys, @files) = @_;
  foreach my $file (@files)
  {
    my $head;
    my $la;
    if($file =~ /[-_](.._..)\.po$/ || $file =~ /^(?:.*\/)?(.._..)\.po$/ ||
    $file =~ /[-_](..)\.po$/ || $file =~ /^(?:.*\/)?(..)\.po$/)
    {
      $la = $1;
      $head = "# translation into language $la file $file\n";
    }
    elsif($file =~ /\.pot$/)
    {
      $la = "en";
      $head = "# template file $file\n";
    }
    else
    {
      die "Language for file $file unknown.";
    }
    die "Could not open outfile $file\n" if !open FILE,">:utf8",$file;
    print FILE "${head}msgid \"\"\nmsgstr \"\"\n";
    my %k;
    foreach my $k (keys %{$keys->{$la}}) { $k{$k} = $keys->{$la}{$k}; }
    foreach my $k (keys %defkeys) { $k{$k} = $defkeys{$k}; }
    foreach my $k (sort keys %k)
    {
      print FILE "\"$k: $k{$k}\\n\"\n";
    }
    print FILE "\n";

    foreach my $en (sort keys %{$data})
    {
      my $ctx;
      my $ennc = $en;
      $ctx = $1 if $ennc =~ s/^___(.*)___//;
      my $str = ($la ne "en" && exists($data->{$en}{$la})) ? $data->{$en}{$la} : "";
      if($data->{$en}{_file})
      {
        foreach my $f (split ";",$data->{$en}{_file})
        {
          print FILE "#: $f\n"
        }
      }
      else
      {
        next;
        # print FILE "#: unknown:0\n"
      }
      print FILE "msgctxt \"$ctx\"\n" if $ctx;
      print FILE "msgid \"$ennc\"\n";
      print FILE "msgid_plural \"$data->{$en}{\"en.1\"}\"\n" if $data->{$en}{"en.1"};
      if($la ne "en" && $data->{$en}{"$la.1"})
      {
        print FILE "msgstr[0] \"$str\"\n";
        print FILE "msgstr[1] \"$data->{$en}{\"$la.1\"}\"\n" if $data->{$en}{"$la.1"};
        print FILE "msgstr[2] \"$data->{$en}{\"$la.2\"}\"\n" if $data->{$en}{"$la.2"};
        print FILE "msgstr[3] \"$data->{$en}{\"$la.3\"}\"\n" if $data->{$en}{"$la.3"};
      }
      else
      {
        print FILE "msgstr \"$str\"\n";
      }
      print FILE "\n";
    }
    close FILE;
  }
}

sub maketxt($)
{
  my ($str) = @_;
  $str =~ s/&gt;/>/g;
  $str =~ s/&lt;/</g;
  $str =~ s/"/\\"/g;
  $str =~ s/&quot;/\\"/g;
  $str =~ s/&apos;/'/g;
  $str =~ s/&amp;/&/g;
  $str =~ s/\n/\\n/g;
  return $str;
}

sub makexml($)
{
  my ($str) = @_;
  $str =~ s/&/&amp;/g;
  $str =~ s/</&lt;/g;
  $str =~ s/>/&gt;/g;
  $str =~ s/\\"/&quot;/g;
  $str =~ s/'/&apos;/g;
  $str =~ s/\\n/\n/g;
  return $str;
}

sub getlang($)
{
  my ($l) = @_;
  if($l eq "ru_RU") {$l = "ru";}
  elsif($l eq "pl_PL") {$l = "pl";}
  return $l;
}

sub replacemat($$$$)
{
  my ($start,$en,$end,$data) = @_;
  $en = maketxt($en);
  my $repl = "$start<desCRIPtion locale=\"en\" >".makexml($en)."</desCRIPtion>$end";
  foreach my $l (sort keys %{$data->{$en}})
  {
    next if $l =~ /[._]/;
    $repl .= "$start<desCRIPtion locale=\"$l\" >".makexml($data->{$en}{$l})."</desCRIPtion>$end";
  }
  return $repl;
}

sub createmat($@)
{
  my ($data, @files) = @_;

  foreach my $file (@files)
  {
    my $x = $/;
    undef $/;
    die "Could not open $file\n" if !open FILE,"<:utf8",$file;
    my $content = <FILE>;
    close FILE;
    foreach my $en (keys %{$data})
    {
      my $ostr = qr/( *)<description +locale="en" *>(.*)<\/description>([\r\n]+)(?: *<description .*\/description>[\r\n]+)*/;
      $content =~ s/$ostr/replacemat($1,$2,$3,$data)/eg;
    }
    if($content =~ /(<description .*)/)
    {
      die "Could not handle string $1.";
    }

    $content =~ s/desCRIPtion/description/g;

    die "Could not open output $file\n" if !open FILE,">:utf8",$file;
    print FILE $content;
    close FILE;
  }
}

sub makenumerus($$$$)
{
  my ($data, $first, $last,$l) = @_;
  my $repl = $first.makexml($data->{$l}).$last;
  for($i = 1; exists($data->{"$l.$i"}); ++$i)
  {
    $repl .= "\n".$first.makexml($data->{"$l.$i"}).$last;
  }
  return $repl;
}

sub convert_ts_message($$$$)
{
  my ($content,$data,$l,$ctx) = @_;
  $content =~ /<source>(.*)<\/source>/s;
  my $source = ($ctx ? "___${ctx}___" : "") .maketxt($1);
  if(exists($data->{$source}{$l}))
  {
    if($content =~ /numerus="yes"/)
    {
      $content =~ s/( +<numerusform>).*(<\/numerusform>)/makenumerus($data->{$source},$1,$2,$l)/se;
    }
    else
    {
      my $repl = makexml($data->{$source}{$l});
      $content =~ s/(<translation).*(<\/translation>)/$1>$repl$2/;
    }
  }
  return $content;
}

sub convert_ts_context($$$)
{
  my ($content,$data,$l) = @_;
  my $ctx;
  $ctx = $1 if(!$nocontext && $content =~ /<name>(.*)<\/name>/);
  $content =~ s/(<message.*?>.*?<\/message>)/convert_ts_message($1,$data,$l,$ctx)/seg;
  return $content;
}

sub createts($@)
{
  my ($data, @files) = @_;

  foreach my $file (@files)
  {
    my $x = $/;
    undef $/;
    die "Could not open $file\n" if !open FILE,"<:utf8",$file;
    my $content = <FILE>;
    close FILE;
    if(!($content =~ /<TS .* language="(.*)">/))
    {
      die "Could not find language for $file.";
    }
    my $l = getlang($1);
    $content =~ s/(<context>.*?<\/context>)/convert_ts_context($1,$data,$l)/seg;

    die "Could not open output $file\n" if !open FILE,">:utf8",$file;
    print FILE $content;
    close FILE;
  }
}

sub main
{
  my %lang;
  my @mat;
  my @po;
  my @ts;
  my $basename = shift @ARGV;
  foreach my $f (@ARGV)
  {
    if($f =~ /\*/) { printf "Skipping $f\n"; }
    elsif($f =~ /\.mat$/) { push(@mat, $f); }
    elsif($f =~ /\.po$/) { push(@po, $f); }
    elsif($f =~ /\.ts$/) { push(@ts, $f); }
    else { die "unknown file extsion."; }
  }
  my %data = loadfiles(\%lang,\%pokeys, @mat,@ts,@po);
  my @cpo;
  foreach my $la (sort keys %lang)
  {
    push(@cpo, "${basename}_$la.po");
  }
  push(@cpo, "$basename.pot");
  die "There have been warning. No output.\n" if $waswarn;
  createpos(\%data, \%pokeys, @cpo);

  createmat(\%data, @mat) if @mat;
  createts(\%data, @ts) if @ts;
}
