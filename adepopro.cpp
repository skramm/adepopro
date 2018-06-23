/**
\file adepopro.cpp

- Author: S. Kramm - 2018
- Licence: GPL v3.0
- hosting: https://github.com/skramm/adepopro

Command-line options:
 - "-s" : module report will be separated by semester (encoded as 5th character of module name)
 - "-p" : shows input file parameters and quits.

 \todo fix the count of characters when utf8 (or other ?) encoding
*/

#include <vector>
#include <ctime>
#include <array>
#include <map>
#include <set>
#include <cassert>
#include <algorithm>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

/// global Output Char Separator for csv files
char g_ocs = ';';
//-------------------------------------------------------------------
enum EN_WeekDay { WD_LUN, WD_MAR, WD_MER, WD_JEU, WD_VEN };

std::map<std::string,EN_WeekDay> g_daymap = {
	{ "Lundi",    WD_LUN },
	{ "Mardi",    WD_MAR },
	{ "Mercredi", WD_MER },
	{ "Jeudi",    WD_JEU },
	{ "Vendredi", WD_VEN }
};

//-------------------------------------------------------------------
/// General string tokenizer, taken from http://stackoverflow.com/a/236803/193789
/**
- see also this one: http://stackoverflow.com/a/53878/193789
*/
std::vector<std::string>
split_string( const std::string &s, char delim )
{
	std::vector<std::string> velems;
//	std::stringstream ss( TrimSpaces(s) );
	std::stringstream ss( s );
	std::string item;
	while( std::getline( ss, item, delim ) )
		velems.push_back(item);


	if( s.back() == delim )                 // add empty field if last char is the delim
		velems.push_back( std::string() );
	return velems;
}
//-------------------------------------------------------------------
/// Returns week index from "Semaine 8"
int
getWeekNum( std::string in )
{
	assert( !in.empty() );
	auto v = split_string( in, ' ' );
	if( v.size() != 2 )
		std::cout << "in=" << in << '\n';
	assert( v.size() == 2 );
	return std::stoi( v[1] );
}
//-------------------------------------------------------------------
EN_WeekDay
getWeekDay( std::string in )
{
	assert( !in.empty() );
	auto v = split_string( in, ' ' );
	assert( v.size() == 2 );
	auto it = g_daymap.find( v[0]  );
	assert( it != std::end(g_daymap) );

	return it->second;
}
//-------------------------------------------------------------------
/// Converts a string "03h00" into a floating point value 3.0
/// and "01h30" into 1.5
float
getDuration( std::string in )
{
	auto v = split_string( in, 'h' );
	assert( v.size() == 2 );
	auto h = std::stoi( v[0] );
	auto mn = std::stoi( v[1] );
	return static_cast<float>( 1.0*h+1.0*mn/60.0 );
}
//-------------------------------------------------------------------
/// Course type: CM/TD/TP
enum EN_Type { TY_CM, TY_TD, TY_TP };

enum EN_PrintSum { PrintSumYes, PrintSumNo };

enum EN_PrintEqTd { PrintEqTdYes, PrintEqTdNo };

/// A triplet of course durations (see EN_Type)
struct Triplet
{
private:
	float _vol[3];

public:
	explicit Triplet()
	{
		clear();
	}
	Triplet( EN_Type ty, float duration )
	{
		clear();
		_vol[ty] = duration;
	}
	Triplet& operator += ( const Triplet& t )
	{
		this->_vol[0] += t._vol[0];
		this->_vol[1] += t._vol[1];
		this->_vol[2] += t._vol[2];
		return *this;
	}
	float sum() const
	{
		return _vol[0] + _vol[1] + _vol[2];
	}
	float sumEqTD() const
	{
		return _vol[0]*3/2 + _vol[1] + _vol[2]*2/3;
	}
	friend std::ostream& operator << ( std::ostream& f, const Triplet& tri )
	{
		f << tri._vol[0] << g_ocs << tri._vol[1] << g_ocs << tri._vol[2] << g_ocs << tri.sum();
		return f;
	}
	void printAsText( std::ostream& f ) const
	{
		f << "CM: " << _vol[0] << " h. - TD: " << _vol[1] << " h. - TP: " << _vol[2] << " h., total: "
			<< sum() << " h., heqTD: " << sumEqTD() << " h.";
	}
	void printTabulated( std::ostream& f, int tab_size, EN_PrintSum psum=PrintSumNo, EN_PrintEqTd peqtd=PrintEqTdNo ) const
	{
#if 0
		f << std::setw(tab_size) << _vol[0]
			<< std::setw(tab_size) << _vol[1]
			<< std::setw(tab_size) << _vol[2];
#else
		std::ostringstream oss;
		oss << "%" << tab_size << ".1f";
		f <<   boost::format( oss.str() ) % _vol[0]
			<< boost::format( oss.str() ) % _vol[1]
			<< boost::format( oss.str() ) % _vol[2];
#endif // 0
		if( psum == PrintSumYes )
		{
			f << "  total = " << sum() << " h.";
		}
		if( peqtd == PrintEqTdYes )
		{
			f << " heqTD = " << sumEqTD() << " h.";
		}
	}
	void clear()
	{
		_vol[0] = _vol[1] = _vol[2] = 0.0f;
	}
};

//-------------------------------------------------------------------
/// Returns type of course and course code from string
/**
The course code embeds the type: CM/TD/TP, coded as last character.
For example:
 - \c ABC1234D => D means TD, will return (TY_TD,ABC1234)
 - \c ABC1234C => C means CM
 - \c ABC1234P => P means TP
*/
std::pair<EN_Type,std::string>
getTypeModule( std::string in )
{
	assert( in.size()>1 );
	EN_Type ty;
	switch( in.back() )
	{
		case 'C': ty = TY_CM; break;
		case 'D': ty = TY_TD; break;
		case 'P': ty = TY_TP; break;
		default: assert(0);
	}
	std::string module = in.substr( 0, in.size()-1 );
	return std::make_pair( ty, module );
}
//-------------------------------------------------------------------
/// Holds the data associated to an instructor or a course module
struct ResourceData
{
	size_t      _nbDays  = 0;   ///< nb days occurency
	size_t      _nbWeeks = 0;   ///< nb weeks occurency
	Triplet     _volume;        ///< teaching volume

/// If instructor data, this will hold the nb of modules he has been affected to
/// If module data, this will hold the nb of instructors on this module
	size_t      _nbOtherResources = 0;

	void incrementDays( size_t n )
	{
		_nbDays += n;
	}
	friend std::ostream& operator << ( std::ostream& f, const ResourceData& ins )
	{
		f << ins._nbDays << g_ocs << ins._nbWeeks << g_ocs << ins._volume << g_ocs << ins._nbOtherResources;
		return f;
	}
};
//-------------------------------------------------------------------
std::ofstream
openFile( std::string fn, std::string text )
{
	std::ofstream file( fn );
	if( !file.is_open() )
		throw std::runtime_error( "Error, unable to open file " + fn );

	std::cout << " - génération du fichier " << fn << '\n';
	file << "# generated by: AdePoPro, see https://github.com/skramm/adepopro"
		<< "\n# generated on: ";
	auto time = std::time(nullptr);
	file << std::put_time(std::localtime(&time), "%F %T%z");

	std::string rule = "\n**************************************\n";
	file << "\n" << rule << "     " << text << rule ;

	return file;
}

/// Used for counting on which days and week a resource is used
typedef
	std::map<
		std::string,  // name
		std::map<
			size_t,   // week number
			std::set<EN_WeekDay>
		>
	>
	ResourceDays;

typedef std::map<std::string,Triplet> TripletMap;

/// Used to store the volume associated to resources
typedef std::map<std::string,TripletMap> ResourceVolumeMap;

/// Used to stored the data associated to a key (which can be an instructor or a course module)
typedef	std::map<std::string,ResourceData> ResourceDataMap;

//-------------------------------------------------------------------
void
process(
	const ResourceVolumeMap& ressVol,  ///< input data
	ResourceDataMap&         data      ///< output data
)
{
		for( const auto& elem: ressVol )
		{
			Triplet tot;
			size_t nb = 0;
			for( const auto& elem2: elem.second )
			{
				tot += elem2.second;
				nb++;
			}

			data[elem.first]._volume = tot;
			data[elem.first]._nbOtherResources = nb;
		}
}
//-------------------------------------------------------------------
void
printChars( std::ostream& f, char c, int nb )
{
	for( int i=0; i<nb; i++ )
		f << c;
}
//-------------------------------------------------------------------
void
printString( std::ofstream& f, std::string str, size_t size_max )
{
	assert( size_max >= str.size() );
	f << str;
	printChars( f, ' ', size_max-str.size() );
}
//-------------------------------------------------------------------
Triplet
printTripletMap( std::ofstream& file, const TripletMap& tmap, size_t max_first )
{
	int tab_size = 6;
	Triplet sum;
	printChars( file, ' ', 8+max_first );
	file << "CM    TD    TP\n";
	for( const auto& elem: tmap )
	{
//		file << "  - " << elemStr << ": ";
		file << "  - ";
		printString( file, elem.first, max_first );
		file << ": ";
		elem.second.printTabulated( file, tab_size );
		file << '\n';
		sum += elem.second;
	}
	file << "- TOTAL: ";
	printChars( file, ' ', max_first-3 );
	sum.printTabulated( file, tab_size, PrintSumYes, PrintEqTdYes );
	file << "\n\n";
	return sum;
}
//-------------------------------------------------------------------
/// Distributes the elements in \c rvm in several maps of same type, based on the value of character \c groupKey
std::map<char,ResourceVolumeMap>
distributeMap( const ResourceVolumeMap& rvm, int groupKey )
{
	std::map<char,ResourceVolumeMap> out;
	for( const auto& elem: rvm )
	{
		auto index_char = elem.first.substr( groupKey, 1 ).at(0);
		auto& current = out[index_char];
		current[elem.first] = elem.second;
	}
	return out;
}
//-------------------------------------------------------------------
/// Describes the fields we want to read in the input file
enum ColIndex : char { CI_Week, CI_Day, CI_Duration, CI_Instructor, CI_Module };

//-------------------------------------------------------------------
/// Unused at present, wil be used to read these in .ini file, see Params
std::map<ColIndex,std::string> g_colIndexStr = {
	{ CI_Week,       "colSemaine" },
	{ CI_Day,        "colJour"    },
	{ CI_Duration,   "colDuree"   },
	{ CI_Instructor, "colEns"     },
	{ CI_Module,     "colModule"  }
};
//-------------------------------------------------------------------
/// Holds all the runtime parameters (fields indexes of input file, ...)
struct Params
{
	char delimiter_in = ';';
	char commentChar = '#';
	std::map<ColIndex,int> colIndex;
	int groupKey_1_pos = 4; ///< char position of sorting level 1
	int groupKey_2_pos = 5; ///< char position of sorting level 2
	bool groupKey_1 = false;
	bool groupKey_2 = false;

	private:
		boost::property_tree::ptree _ptree;

	public:
	/// constructor, calls main constructor an tries to read .ini file
	Params( std::string filename ): Params()
	{
		bool hasIniFile = true;
		try
		{
			boost::property_tree::ini_parser::read_ini( filename, _ptree );
		}
		catch( const std::exception& e )
		{
			std::cerr << "No file " << filename << ", keeping defaults\n";
			hasIniFile = false;
		}
		if( hasIniFile )
		{
			std::cout << "reading data in config file " << filename << '\n';
			groupKey_1 = (bool)_ptree.get<int>( "grouping.groupKey1", groupKey_1 );
			groupKey_2 = (bool)_ptree.get<int>( "grouping.groupKey2", groupKey_2 );
			groupKey_1_pos = _ptree.get<int>( "grouping.groupKey1_pos", groupKey_1_pos );
			groupKey_2_pos = _ptree.get<int>( "grouping.groupKey2_pos", groupKey_2_pos );
			for( const auto& map_key: g_colIndexStr)
				colIndex[map_key.first] = _ptree.get<int>( "columns." + map_key.second, colIndex[map_key.first] );
		}
	}
	/// Constructor, assigns default values
	Params()
	{
		colIndex[CI_Week]       = 0;
		colIndex[CI_Day]        = 1;
		colIndex[CI_Duration]   = 2;
		colIndex[CI_Instructor] = 6;
		colIndex[CI_Module]     = 8;
	}
	size_t getHighestIndex() const
	{
		return std::max_element(
			std::begin(colIndex),
			std::end(colIndex),
			[]     // lambda
			( const std::pair<ColIndex,int> & p1, const std::pair<ColIndex,int> & p2 )
			{
				return p1.second < p2.second;
			}
		)->second;
	}

	friend std::ostream& operator << ( std::ostream& f, const Params& p )
	{
		f << "Input File parameters:"
			<< "\n -delimiter='"   << p.delimiter_in   << '\''
			<< "\n -commentChar='" << p.commentChar << '\''
			<< "\n - grouping:" << std::boolalpha
			<< "\n  - key1: " << p.groupKey_1 << " pos=" << p.groupKey_1_pos
			<< "\n  - key2: " << p.groupKey_2 << " pos=" << p.groupKey_2_pos
			<< "\n -input file indexes:\n";

		for( size_t i=0; i<g_colIndexStr.size(); i++ )
			f << "  -" << g_colIndexStr[(ColIndex)i] << ": " << p.colIndex.at((ColIndex)i) << '\n';
		f << " -highest index = " << p.getHighestIndex() << '\n';
		return f;
	}
};
//-------------------------------------------------------------------
/// Returns the max size of the string in key
size_t
findMaxStringSize( const ResourceVolumeMap& rvm )
{
	size_t res=0;
	for( const auto& elem1: rvm )
	{
		using PairType = std::pair<std::string,Triplet>;
		const auto it = std::max_element(
			std::begin( elem1.second ),
			std::end(   elem1.second ),
			[]                                               // lambda
			( const PairType& p1, const PairType& p2 )
			{
				return p1.first.size() < p2.first.size();
			}
		);
//		std::cout << "it:" << it->first;
		res = std::max( res, it->first.size() );
	}
	return res;
}
//-------------------------------------------------------------------
//template<typename T>
Triplet
printMap( std::ofstream& file, const std::map<std::string,TripletMap>& mapData, std::string text, size_t max_size )
{
	Triplet sum;
	for( const auto& elem: mapData )
	{
		file << "- " << text << ": " << elem.first << '\n';
		auto s = printTripletMap( file, elem.second, max_size );
		sum += s;
	}
	return sum;
}
//-------------------------------------------------------------------
/// Holds all the data read from the file, along with the processing functions
struct Data
{
	ResourceDataMap _instructorData;  ///< key: instructor name
	ResourceDataMap _moduleData;      ///< key: module name

	ResourceDays _instructorDays;
	ResourceDays _moduleDays;

	ResourceVolumeMap _mod_prof;
	ResourceVolumeMap _prof_mod;

/// Add one event to the data
	void addOne( std::string instr, size_t num_sem, EN_WeekDay wd, const std::pair<EN_Type,std::string>& type_mod, float duration )
	{
		assert( !instr.empty() );

		auto type   = type_mod.first;
		auto module = type_mod.second;

		{
			auto& ref_inst = _instructorDays[instr];
			auto& ref_week = ref_inst[num_sem];
			ref_week.insert( wd );
		}

		{
			auto& ref_mod = _moduleDays[module];
			auto& ref_week = ref_mod[num_sem];
			ref_week.insert( wd );
		}

		auto tri = Triplet( type, duration );

		auto& mod1 = _mod_prof[module];
		auto& pr1  =  mod1[instr];
		pr1 += tri;

		auto& pr2  = _prof_mod[instr];
		auto& mod2 =  pr2[module];
		mod2 += tri;
	}

	void compute()
	{
		for( const auto& elem: _instructorDays )
		{
			for( const auto& sem: elem.second )
				_instructorData[elem.first].incrementDays( sem.second.size() );
			_instructorData[elem.first]._nbWeeks = elem.second.size();
		}

		for( const auto& elem: _moduleDays )
		{
			for( const auto& sem: elem.second )
				_moduleData[elem.first].incrementDays( sem.second.size() );
			_moduleData[elem.first]._nbWeeks = elem.second.size();
		}

		process( _prof_mod, _instructorData );
		process( _mod_prof, _moduleData );
	}

/// write report of Modules / Instructor
	void writeReport_MI( std::string fn, bool sortLevel_1, bool sortLevel_2, const Params& params )
	{
		auto file = openFile( fn, "Bilan module/enseignant" );

		auto max_size = findMaxStringSize( _mod_prof );

		Triplet bigsum;
		if( sortLevel_1 )
		{
			auto mapLevel_1 = distributeMap( _mod_prof, params.groupKey_1_pos );
			for( const auto& elem1: mapLevel_1 )
			{
				Triplet sumLevel_1;
				file << "*** SEMESTRE " << elem1.first << " ***\n\n";
				const auto current = elem1.second;
				if( sortLevel_2 )
				{
					Triplet sumLevel_2;
					auto mapLevel2 = distributeMap( current, params.groupKey_2_pos );

					for( const auto& elem1: mapLevel2 )
					{
						file << "** UE " << elem1.first << " **\n\n";
						auto tot = printMap( file, elem1.second, "module", max_size );
						file << "* Total UE " << elem1.first << ": ";
						tot.printAsText( file );
						file << "\n\n";
						bigsum     += tot;
						sumLevel_1 += tot;
					}
				}
				else
				{
					auto tot = printMap( file, current, "module", max_size );
					bigsum     += tot;
					sumLevel_1 += tot;
				}
				file << "* Total semestre " << elem1.first << ": ";
				sumLevel_1.printAsText( file );
				file << "\n\n";
			}
		}
		else
		{
			auto tot = printMap( file, _mod_prof, "module", max_size );
			bigsum  += tot;
		}
		file << "\n*** TOTAL GENERAL ***\n";
		bigsum.printAsText( file );
		file << "\n";
	}

/// write report of Instructor / Modules
	void writeReport_IM( std::string fn )
	{
		auto file = openFile( fn, "Bilan enseignant/module" );

		auto max_size = findMaxStringSize( _prof_mod );
//		std::cout << __FUNCTION__ << "(): max_size=" << max_size << '\n';

		Triplet bigsum;
		for( const auto& elem1: _prof_mod )
		{
			file << "Enseignant:" << elem1.first << '\n';
			bigsum += printTripletMap( file, elem1.second, max_size );
		}
		file << "\n*** TOTAL GENERAL ***\n" << bigsum << '\n';
	}

/// write CSV data
	void writeCsv( std::string fn, const ResourceDataMap& dataMap, std::string headline )
	{
		auto file = openFile( fn, headline );
		for( const auto& elem: dataMap )
		{
			auto data = elem.second;
			file << elem.first << g_ocs << data << g_ocs
				<< 1.0*data._volume.sum() / data._nbDays
				<< '\n';
		}
	}
};

//-------------------------------------------------------------------
/// see adepopro.cpp
int main( int argc, char* argv[] )
{
	if( argc < 2 )
	{
		std::cout << "usage: " << argv[0] << " <input_csv_file>\n";
		return 1;
	}
	std::string fn_in = argv[argc-1];
	Params params( "adepopro.ini" ); // attemps to read in file, else keeps default values

	bool printOptions = false;
	if( argc > 1 )
	{
		for( int i=1; i<argc; i++ )
		{
			if( std::string(argv[i]) == "-s" )
				params.groupKey_1_pos = true;
			if( std::string(argv[i]) == "-p" )
				printOptions = true;
		}
	}
	if( printOptions )
	{
		std::cout << params << '\n';
		return 1;
	}

	std::ifstream file( fn_in );
	if( !file.is_open() )
		throw std::runtime_error( "Error, unable to open file " + fn_in );

	Data results;

	std::string buff;
	int line = 0;
	while ( getline (file, buff ) )
	{
		line++;
//		std::cout << "line " << line << ": " << buff << '\n';

		auto v_str = split_string( buff, params.delimiter_in );
		if( v_str.size() < params.getHighestIndex() && !v_str.empty() )
		{
			std::cout << "erreur, champ manquants: " << v_str.size() << " au lieu de " << params.getHighestIndex() << " au minimum:\n" << buff << '\n';
			for( auto s: v_str )
				std::cout << "-" << s << '\n';
			throw std::runtime_error("error");
		}
		if( !v_str.empty() )
		if( !v_str[0].empty() && v_str[0].front() != params.commentChar )
		{
			auto week_num = getWeekNum(  v_str.at(params.colIndex[CI_Week])      );
			auto weekday  = getWeekDay(  v_str.at(params.colIndex[CI_Day])       );
			auto duration = getDuration( v_str.at(params.colIndex[CI_Duration])  );
			auto name     =              v_str.at(params.colIndex[CI_Instructor] );
			auto code     =              v_str.at(params.colIndex[CI_Module]     );

			if( !code.empty() )
			{
//				std::cout << "buff=" << buff << '\n';
//				std::cout << "line=" << line << " nb champs=" << v_str.size() << " name=" << name << " day=" << (int)weekday << " weeknum=" << week_num << " code=" << code << '\n';
				auto type_mod = getTypeModule( code );
//				if( v_str.size() > 9 )
//					std::cout << "line " << line << ": " << buff << '\n';
				if( name.empty() )
					name = "(néant)";
				results.addOne( name, week_num, weekday, type_mod, duration );
			}
		}
	}
	results.compute();
	auto fn2 = split_string( fn_in, '.' );
	assert( fn2.size() > 0 );

// csv output file headers
	std::string head1 = "# Nom;Nb jours;Nb sem;vol. CM;vol. TD;vol. TP;vol. total;";
	std::string head2 = ";ratio vol. total/nb jours";

	results.writeCsv( "adepopro_E_" + fn2[0] + ".csv", results._instructorData, head1 + "nb modules"     + head2 );
	results.writeCsv( "adepopro_M_" + fn2[0] + ".csv", results._moduleData,     head1 + "nb enseignants" + head2 );
	results.writeReport_MI( "adepopro_ME_" + fn2[0] + ".txt", params.groupKey_1, params.groupKey_2, params );
	results.writeReport_IM( "adepopro_EM_" + fn2[0] + ".txt" );

}
//-------------------------------------------------------------------

