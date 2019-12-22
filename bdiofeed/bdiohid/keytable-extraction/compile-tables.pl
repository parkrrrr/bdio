#!/usr/bin/perl

# ttydriver "XXX";
# ttymodel "YYY";
# ttycode "ZZ";
# manufacturer "XXX";
# model "YYY";
# display {
#   row {
#     cells 8;
#     router1 {
#       router GRP; # name 
#     }
#   }
#   button NUM GRP ID "name";
#   # untranslated key group GRP "name"
# }

my $brltty_path = '/home/ron/brltty';

my $driver = undef;
my $model = undef;
my $code = undef;
my $title = undef;
my @entries = undef;
my @leftEntries = undef;
my @rightEntries = undef;

my %mapping = undef;
my $notes = '';
my %mapvars = undef;
my %bindings = undef;

sub AddBindings
{
    my $arg = shift(@_);

    # remove the ! that serves as a flag to brlapi
    $arg =~ s/!//g;

    @args = split /\+/, $arg;
    foreach $myArg (@args)
    {
        $bindings{$myArg} = $arg;
    }
}

sub ReadMappingFile
{
    my $filename = shift(@_);

    my $parentFolder = "$brltty_path/Tables/Input/";
    my $mappingFolder = "$parentFolder/$code/";
    my $mapFilename = "$mappingFolder/$filename";

    if ($filename =~ m/^\.\.\/(.*)/ )
    {
        $mapFilename = "$parentFolder/$filename";
    }

    open my $mappingFile, '<', $mapFilename or return;
    while (<$mappingFile>)
    {
        chomp;

        # translate e.g. Left\{navKeyType}Press to LeftJoystickPress based on assignments we've seen.
        s/\\\{([^}]+)\}/$mapvars{$1}/g;

        if (/^title (.*)$/)
        {
            $title = $1;
            $title =~ s/$driver //;
        }
        elsif (/^include (.*)$/)
        {
           ReadMappingFile($1);
        }
        elsif (/^map *([^ ]+) +([^ ]+)$/)
        {
            $mapping{$1} = $2;
            $bindings{$1} = $_;
        }
        elsif (/^bind *([^ ]+) +/)
        {
            AddBindings($1);
        }
        elsif (/^assign ([^ ]*) (.*)$/)
        {
            my $key = $1;
            my $value = $2;
            $value =~ s/\\s/ /g;
            $mapvars{$key} = $value;
        }
        elsif (/^note (.*)$/)
        {
            $notes .= "# $1\n";
        }
    }
    close $mappingFile;
}

sub ReadMappings
{
    %mapping = undef;
    $title = undef;
    $notes = '';
    %mapvars = undef;
    %bindings = undef;

    ReadMappingFile("$model.ktb");
}

sub MapName
{
    my $name = shift(@_);

    my $result = $mapping{$name};

    if ($result)
    {
        if ($result eq 'SPACE' || $result =~ m/^DOT\d$/)
        {
            return lc($result);
        }
    }
    return undef;
}

sub MapMovementKeys
{
    my $name = shift(@_);

    if ($name =~ m/Joystick(.*)/)
    {
        # maps 'LeftJoystickLeft' to 'jleft' with special mapping for press -> jcenter
        if ($1 eq 'Press')
        {
            return "jcenter";
        }
        return lc("j$1");
    }
    if ($name =~ m/Pad(.*)/)
    {
        # maps 'LeftPadRight' to 'dright' 
        return lc("d$1");
    }
    if ($name =~ m/Rocker(.*)/)
    {
        # maps 'LeftRockerUp' to 'rup'
        return lc("r$1");
    }
    if ($name =~ m/Pan(.*)/)
    {
        # maps 'LeftPanLeft' to 'pleft'
        return lc("p$1");
    }
}

sub WriteEntries
{
    my $writeFile = shift(@_);
    my $entries = shift(@_);
    my $I = shift(@_);
    my $collectionName = shift(@_);

    return $I if !$#$entries;

    my $in = '';
    if ($collectionName ne '')
    {
        print $writeFile qq(  $collectionName {\n);
        $in = '  ';
    }

    foreach my $entry (@$entries)
    {
        if ($entry)
        {
            my $group = @$entry[0];
            my $key = @$entry[1];
            my $name = @$entry[2];

            my $bound = $bindings{$name};
            my $bindComment = '';

            if (!$bound)
            {
                $bindComment = '  # brltty has no binding for this key';
            }

            if ($key < 0)
            {
                print $writeFile qq($in  # unrecognized group $group named "$name" $bindComment\n);
            }
            elsif ($name eq 'LeftSpace' || $name eq 'SpaceLeft')
            {
                print $writeFile qq($in  lspace   $group $key; # $name $bindComment\n);
            }
            elsif ($name eq 'RightSpace' || $name eq 'SpaceRight')
            {
                print $writeFile qq($in  rspace   $group $key; # $name $bindComment\n);
            }
            elsif ($mapping = MapName($name))
            {
                print $writeFile qq($in  $mapping    $group $key; # $name $bindComment\n);
            }
            elsif ($mapping = MapMovementKeys($name))
            {
                print $writeFile qq($in  $mapping    $group $key; # $name $bindComment\n);
            }
            elsif ($name =~ m/Dot(\d)/)
            {
                print $writeFile qq($in  dot$1   $group $key; # $name $bindComment\n);
            }
            elsif ($name eq 'Space')
            {
                print $writeFile qq($in  space   $group $key; # $name $bindComment\n);
            }
            else
            {
                print $writeFile qq($in  button $I   $group $key "$name"; $bindComment\n);
                ++$I;
            }
        }
    }
    if ($collectionName ne '')
    {
        print $writeFile qq(  }\n\n);
    }

    return $I;
}

sub CreateFile
{
    ReadMappings;

    open my $writeFile, '>', "mg-definitions/$code-$model" or die "Error writing file";

    if (!$title)
    {
        $title = ucfirst($model);
    }

    print $writeFile qq(# Display definition for $driver : $title\n\n);
    print $writeFile qq($notes\n);
    print $writeFile qq(ttydriver    "$driver"\n);
    print $writeFile qq(ttymodel     "$model"\n);
    print $writeFile qq(ttycode      "$code"\n);
    print $writeFile qq(manufacturer "$driver"\n);
    print $writeFile qq(model        "$title"\n\n);

    print $writeFile qq(display {\n);

    print $writeFile qq(  row {\n);
    print $writeFile qq(    cells 8;\n);
    foreach (@entries)
    {
        if ($_ && @$_[2] =~ m/Routing/)
        {
            my $level = 1;
            if (@$_[2] =~ m/(\d)/)
            {
                $level = $1;
            }
            print $writeFile qq(    router$level {\n);
            print $writeFile qq(      router   @$_[0];  # @$_[2]\n);
            print $writeFile qq(    }\n);
            $_ = undef;
        }
    }
    print $writeFile qq(  }\n\n);

    my $I = 1;
    $I = WriteEntries($writeFile, [@leftEntries], $I, 'left');
    $I = WriteEntries($writeFile, [@rightEntries], $I, 'right');
    $I = WriteEntries($writeFile, [@entries], $I, '');
    print $writeFile qq(}\n);
    close $writeFile;
}

open (ALLTABLES, "<alltables") or die "Can't find alltables";
while (<ALLTABLES>)
{
    chomp;

    if (/DRIVER_CODE = (.*)/)
    {
        $code = $1;
    }
    elsif (/DRIVER_NAME = (.*)/)
    {
        $driver = $1;
    }
    elsif (/table (.*) \{/)
    {
        $model = $1;
    }
    elsif (/entry (.*) (.*) \[(.*)\]/)
    {
        my $group = $1;
        my $key = $2;
        my $name = $3;

        if ($name =~ m/^Left./ && $name ne 'LeftSpace')
        {
            push @leftEntries, [$group, $key, $name];
        }
        elsif ($name =~ m/^Right./ && $name ne 'RightSpace')
        {
            push @rightEntries, [$group, $key, $name];        
        }
        else 
        {
            push @entries, [$1,$2,$3];
        }
    }
    elsif (/\}/)
    {
        CreateFile;
        @entries = undef;
        @leftEntries = undef;
        @rightEntries = undef;
        $model = undef;
    }
}
close ALLTABLES;