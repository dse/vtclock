package Term::VTClock;

our $VERSION = "1.0.0";

use Curses;
use List::Util qw(min max);
use POSIX qw(strftime);

sub new {
    my ($class, @args) = @_;
    my $self = {};
    bless($self, $class);
    $self->init();
    return $self;
}

my $DEFAULT_CHARS;
my $BUILTIN_FONTS;
BEGIN {
    $BUILTIN_FONTS = {
	"seven-segment" => {
	    "0" => [" ___ ",
		    "|   |",
		    "|   |",
		    "|   |",
		    "|___|"],
	    "1" => ["     ",
		    "    |",
		    "    |",
		    "    |",
		    "    |"],
	    "2" => [" ___ ",
		    "    |",
		    " ___|",
		    "|    ",
		    "|___ "],
	    "3" => [" ___ ",
		    "    |",
		    " ___|",
		    "    |",
		    " ___|"],
	    "4" => ["     ",
		    "|   |",
		    "|___|",
		    "    |",
		    "    |"],
	    "5" => [" ___ ",
		    "|    ",
		    "|___ ",
		    "    |",
		    " ___|"],
	    "6" => [" ___ ",
		    "|    ",
		    "|___ ",
		    "|   |",
		    "|___|"],
	    "7" => [" ___ ",
		    "    |",
		    "    |",
		    "    |",
		    "    |"],
	    "8" => [" ___ ",
		    "|   |",
		    "|___|",
		    "|   |",
		    "|___|"],
	    "9" => [" ___ ",
		    "|   |",
		    "|___|",
		    "    |",
		    "    |"],
	    ":" => ["   ",
		    " : ",
		    "   ",
		    " : ",
		    "   "]
	   }
       };
    $DEFAULT_CHARS = {
	"0" => [" ### ",
		"#   #",
		"#  ##",
		"# # #",
		"##  #",
		"#   #",
		" ### "],
	"1" => ["  #  ",
		" ##  ",
		"  #  ",
		"  #  ",
		"  #  ",
		"  #  ",
		" ### "],
	"2" => [" ### ",
		"#   #",
		"    #",
		"   # ",
		"  #  ",
		" #   ",
		"#####"],
	"3" => ["#####",
		"   # ",
		"  #  ",
		"   # ",
		"    #",
		"#   #",
		" ### "],
	"4" => ["   # ",
		"  ## ",
		" # # ",
		"#  # ",
		"#####",
		"   # ",
		"   # "],
	"5" => ["#####",
		"#    ",
		"#### ",
		"    #",
		"    #",
		"#   #",
		" ### "],
	"6" => ["  ## ",
		" #   ",
		"#    ",
		"#### ",
		"#   #",
		"#   #",
		" ### "],
	"7" => ["#####",
		"    #",
		"   # ",
		"  #  ",
		" #   ",
		" #   ",
		" #   "],
	"8" => [" ### ",
		"#   #",
		"#   #",
		" ### ",
		"#   #",
		"#   #",
		" ### "],
	"9" => [" ### ",
		"#   #",
		"#   #",
		" ####",
		"    #",
		"   # ",
		" ##  "],
	":" => [" ",
		" ",
		"#",
		" ",
		"#",
		" ",
		" "],
    };
}

sub init {
    my ($self) = @_;
    $self->{chars} = $DEFAULT_CHARS;
}

sub char_width {
    my ($self, $char) = @_;
    return max map { length($_) } @{$self->{chars}->{$char}};
}

sub char_height {
    my ($self, $char) = @_;
    return scalar @{$self->{chars}->{$char}};
}

sub max_char_width {
    my ($self, @chars) = @_;
    return max map { $self->char_width($_) } @chars;
}

sub max_char_height {
    my ($self, @chars) = @_;
    return max map { $self->char_height($_) } @chars;
}

sub digit_width {
    my ($self) = @_;
    return $self->max_char_width("0" .. "9");
}

sub digit_height {
    my ($self) = @_;
    return $self->max_char_height("0" .. "9");
}

sub colon_width {
    my ($self) = @_;
    return $self->char_width(":");
}

sub colon_height {
    my ($self) = @_;
    return $self->char_height(":");
}

sub make_digit_window {
    my ($self) = @_;
    my $w = $self->digit_width();
    my $h = $self->digit_height();
    my $window = subwin($self->{cl},
			$h, $w + 1,
			$self->{starty},
			$self->{startx});
    $self->{startx} += $w + $self->{space};
    return $window;
}

sub make_colon_window {
    my ($self) = @_;
    my $w = $self->colon_width();
    my $h = $self->colon_height();
    my $window = subwin($self->{cl},
			$h, $w + 1,
			$self->{starty},
			$self->{startx});
    $self->{startx} += $w + $self->{space};
    return $window;
}

sub draw_char {
    my ($self, $w, $char) = @_;
    my $string = join("\n", @{$self->{chars}->{$char}});
    if (defined $self->{char} && length $self->{char}) {
	my $char = substr($self->{char}, 0, 1);
	$string =~ s{\S}{$char}g;
    }
    addstr($w, 0, 0, $string);
}

use Time::HiRes qw(usleep gettimeofday);

sub delay {
    my ($self) = @_;
    my ($sec, $usec) = gettimeofday();
    usleep(1000000 - $usec);
}

sub delay_to_half_second {
    my ($self) = @_;
    my ($sec, $usec) = gettimeofday();
    if ($usec >= 500000) {
	return;
    }
    usleep(500000 - $usec);
}

sub set_figlet_font {
    my ($self, $font) = @_;

    # in OS X, cpan-installed Text::FIGlet won't find fonts from
    # brew-installed figlet package.
    if ($^O eq "darwin") {
	if (-d "/usr/local/share/figlet/fonts") {
	    $ENV{FIGLIB} = "/usr/local/share/figlet/fonts";
	}
    }

    require Text::FIGlet;
    my $figlet = Text::FIGlet->new((defined $font) ? (-f => $font) : ());

    # initialize
    $self->{chars} = {};
    my %char_length;

    foreach my $char ("0" .. "9", ":") {
	my $figify = $figlet->figify(-A => $char);
	my @figify = split("\n", $figify);

	# make sure all lines are the same length, by padding with
	# spaces where needed.
	my $max_length = max map { length $_ } @figify;
	foreach (@figify) {
	    $_ .= " " x ($max_length - length($_));
	}

	$char_length{$char} = $max_length;
    	$self->{chars}->{$char} = \@figify;
    }

    # make sure all digits are the same width, by padding with spaces
    # on both sides as needed.
    my $digit_width = max map { $char_length{$_} } "0".."9";
    print("Digit width: $digit_width\n");

    foreach my $char ("0".."9") {
	foreach (@{$self->{chars}->{$char}}) {
	    $len = $digit_width - length($_);
	    $l = int($len / 2);
	    $r = $len - $l;
	    $_ = (" " x $l) . $_ . (" " x $r);
	}
    }
}

sub run {
    my ($self) = @_;

    initscr();
    cbreak();
    noecho();
    nonl();
    timeout(curscr, 50);

    $SIG{__DIE__} = sub {
	endwin();
	CORE::die(@_);
    };

    # defaults
    $self->{space} //= 1;

    $self->{cl_height} = $self->max_char_height("0" .. "9", ":");
    $self->{cl_width} = $self->digit_width() * 6 + $self->colon_width() * 2 + $self->{space} * 7 + 1;

    $self->{x} = int((COLS()  - $self->{cl_width})  / 2);
    $self->{y} = int((LINES() - $self->{cl_height}) / 2);

    if (LINES() < ($self->{cl_height} + 2) || COLS() < ($self->{cl_width})) {
	endwin();
	die(sprintf("Screen (%d x %d) is too small (minimum is %d x %d).\n",
		    COLS(), LINES(),
		    $self->{cl_width}, $self->{cl_height} + 2
		   ));
    }

    $self->{startx} = $self->{x};
    $self->{starty} = $self->{y};

    $self->{updown}    = (LINES() > $self->{cl_height}) ? 1 : 0;
    $self->{leftright} = (COLS()  > $self->{cl_width})  ? 1 : 0;

    $self->{cl}  = newwin($self->{cl_height}, $self->{cl_width}, $self->{y}, $self->{x});
    $self->{cld} = newwin($self->{cl_height}, $self->{cl_width}, $self->{y}, $self->{x});

    $self->{h1} = $self->make_digit_window();
    $self->{h2} = $self->make_digit_window();
    $self->{c1} = $self->make_colon_window();
    $self->{m1} = $self->make_digit_window();
    $self->{m2} = $self->make_digit_window();
    $self->{c2} = $self->make_colon_window();
    $self->{s1} = $self->make_digit_window();
    $self->{s2} = $self->make_digit_window();

    curs_set(0);

    my $futurex;		# temporary for bounds checking
    my $futurey;
    my $waitfor = 0;		# bouncing-related counter

    while (1) {
	my $time_string = strftime("%H:%M:%S", localtime());

	$self->draw_char($self->{h1}, substr($time_string, 0, 1));
	$self->draw_char($self->{h2}, substr($time_string, 1, 1));
	$self->draw_char($self->{c1}, ":");
	$self->draw_char($self->{m1}, substr($time_string, 3, 1));
	$self->draw_char($self->{m2}, substr($time_string, 4, 1));
	$self->draw_char($self->{c2}, ":");
	$self->draw_char($self->{s1}, substr($time_string, 6, 1));
	$self->draw_char($self->{s2}, substr($time_string, 7, 1));

	if ($self->{bounce}) {
	    if ($waitfor >= $self->{bounce}) {

		# erase old
		mvwin($self->{cld}, $self->{y}, $self->{x});
		noutrefresh($self->{cld});

		$futurex = $self->{x} + $self->{leftright};
		$futurey = $self->{y} + $self->{updown};

		if ($futurex == 0 && $futurey == 0) {
		    $futurex = $self->{x} + ($self->{leftright} *= -1);
		    $futurey = $self->{y} + ($self->{updown}    *= -1);
		} else {
		    if ($futurex < 0 ||
			  $futurex > (COLS() - $self->{cl_width})) {
			$futurex = $self->{x} + ($self->{leftright} *= -1);
		    }
		    if ($futurey < 0 ||
			  $futurey > (LINES() - $self->{cl_height})) {
			$futurey = $self->{y} + ($self->{updown}    *= -1);
		    }
		}

		$self->{x} = $futurex;
		$self->{y} = $futurey;
		$waitfor = 0;
	    }
	}

	mvwin($self->{cl}, $self->{y}, $self->{x});
	noutrefresh($self->{cl});
	doupdate();

	if ($self->{blinking_colons}) {
	    $self->delay_to_half_second();
	    erase($self->{c1});
	    erase($self->{c2});
	    touchwin($self->{cl});
	    noutrefresh($self->{cl});
	    doupdate();
	}

	$self->pollkey();
	$self->delay();
	++$waitfor;
    }

    endwin();
}

sub pollkey {
    my ($self) = @_;
    my $key = getch(curscr);
    if ($key eq chr(12) || $key eq chr(18) || $key eq KEY_REFRESH) {
	redrawwin(curscr);
	refresh(curscr);
    }
}

1;

