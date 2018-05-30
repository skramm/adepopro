/**
\file adepopro.cpp

- Author: S. Kramm - 2018
- Licence: GPL v3.0
- See https://github.com/skramm/adepopro

Command-line option: -s : module report will be separated by semester (encoded as 5th character of module name)
*/

#include <vector>
#include <array>
#include <map>
#include <set>
#include <cassert>
#include <algorithm>
//#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream> // needed for expansion of SPAG_LOG

/// global Output Char Separator
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
	friend std::ostream& operator << ( std::ostream& f, const Triplet& tri )
	{
		f << tri._vol[0] << g_ocs << tri._vol[1] << g_ocs << tri._vol[2] << g_ocs << tri.sum();
		return f;
	}
private:
	void clear()
	{
		_vol[0] = _vol[1] = _vol[2] = 0.0f;
	}
};

/// Returns type of course and course code from string
/**
The course code embeds the type: CM/TD/TP, coded as last character.
For example:
 - \c ABC1234D => D means TD
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
struct ResourceData
{
	size_t      _nbDays  = 0;   ///< nb days presence
	size_t      _nbWeeks = 0;   ///< nb weeks presence
	Triplet     _volume;       ///< teaching volume
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
	file << text;

	return file;
}

/// Used for counting on which days a resource is used
typedef
	std::map<
		std::string,  // name
		std::map<
			size_t,   // week number
			std::set<EN_WeekDay>
		>
	>
	ResourceDays;

/// Used to store the volume associated to resources
typedef
	std::map<
		std::string,      // key1: resource 1
		std::map<
			std::string,  // key2: resource 2
			Triplet
		>
	>
	ResourcesVolume;

//-------------------------------------------------------------------
void
process(
	const ResourcesVolume&              ressVol,  ///< input data
	std::map<std::string,ResourceData>& data      ///< output data
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
struct Results
{
	std::map<std::string,ResourceData> _instructorData;  ///< key: instructor name
	std::map<std::string,ResourceData> _moduleData;      ///< key: module name

	ResourceDays _instructorDays;
	ResourceDays _moduleDays;

	ResourcesVolume _mod_prof;
	ResourcesVolume _prof_mod;

/// Add one event to the data
	void addOne( std::string name, size_t num_sem, EN_WeekDay wd, const std::pair<EN_Type,std::string>& type_mod, float duration )
	{
		assert( !name.empty() );

		auto type   = type_mod.first;
		auto module = type_mod.second;

		{
			auto& ref_inst = _instructorDays[name];
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
		auto& pr1  =  mod1[name];
		pr1 += tri;

		auto& pr2  = _prof_mod[name];
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
	void writeReport_MI( std::string fn, bool sortBySemester )
	{
		auto file = openFile( fn, "Bilan module/enseignant" );

		char current_semestre = ' ';
		for( const auto& mod: _mod_prof )
		{
			auto module = mod.first;
			auto semestre = module.substr( 4, 1 ).at(0);
			if( current_semestre != semestre && sortBySemester )
			{
				file << "\n *** SEMESTRE " << semestre << " *** \n\n";
				current_semestre = semestre;
			}
			file << "- module:" << module << '\n';
			Triplet tot;
			for( const auto& prof: mod.second )
			{
				file << "  - prof:" << prof.first << ": " << prof.second << '\n';
				tot += prof.second;
			}
			file << "- TOTAL=" << tot << '\n';
		}
	}
/// write report of Instructor / Modules
	void writeReport_IM( std::string fn )
	{
		auto file = openFile( fn, "Bilan enseignant/module" );

		for( const auto& inst: _prof_mod )
		{
			file << "\nEnseignant:" << inst.first << '\n';
			Triplet tot;
			for( const auto& mod: inst.second )
			{
				file << "  - module: " << mod.first << ": " <<  mod.second << '\n';
				tot += mod.second;
			}
			file << "- TOTAL=" << tot << '\n';
		}
	}
/// write CSV data of Instructors
	void writeCsv_I( std::string fn, std::string headline )
	{
		auto file = openFile( fn, headline );
		for( const auto& instr: _instructorData )
		{
			auto instrData = instr.second;
			file << instr.first << g_ocs << instrData << g_ocs
				<< 1.0*instrData._volume.sum() / instrData._nbDays
				<< '\n';
		}
	}

/// write CSV data of Modules
	void writeCsv_M( std::string fn, std::string headline )
	{
		auto file = openFile( fn, headline );
		for( const auto& module: _moduleData )
		{
			auto modData = module.second;
			file << module.first << g_ocs << modData << g_ocs
				<< 1.0*modData._volume.sum() / modData._nbDays
				<< '\n';
		}
	}

};
//-------------------------------------------------------------------
/// Describes the fields we want to read in the input file
enum ColIndex : char { CI_Week, CI_Day, CI_Duration, CI_Instructor, CI_Module, CI_NB_ELEMS };

#if 0
/// Unused at present, wil be used to read these in .ini file, see InputFormat
std::map<ColIndex,std::string> g_colIndexStr = {
	{ CI_Week,       "colSemaine" },
	{ CI_Day,        "colJour"    },
	{ CI_Duration,   "colDuree"   },
	{ CI_Instructor, "colEns"     },
	{ CI_Module,     "colModule"  }
};
#endif
//-------------------------------------------------------------------
/// Holds the fields indexes of input file
/// \todo add reading of these in a .ini file
struct InputFormat
{
	char delimiter = ';';
	std::array<int,CI_NB_ELEMS> colIndex;

	InputFormat()
	{
		colIndex[CI_Week]       = 0;
		colIndex[CI_Day]        = 1;
		colIndex[CI_Duration]   = 2;
		colIndex[CI_Instructor] = 6;
		colIndex[CI_Module]     = 8;
	}
	size_t getHighestIndex() const
	{
		return *std::max_element( std::begin(colIndex), std::end(colIndex) );
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

	bool sortBySemester = false;
	if( argc > 2 )
		if( std::string(argv[1]) == "-s" )
			sortBySemester = true;

	std::ifstream file( fn_in );
	if( !file.is_open() )
		throw std::runtime_error( "Error, unable to open file " + fn_in );

	Results results;

	InputFormat inputFormat;
	std::string buff;
	int line = 0;
	while ( getline (file, buff ) )
	{
		line++;
//		std::cout << "line " << line << ": " << buff << '\n';

		auto v_str = split_string( buff, inputFormat.delimiter );
		if( v_str.size() < inputFormat.getHighestIndex() && !v_str.empty() )
		{
			std::cout << "erreur, champ manquants: " << v_str.size() << " au lieu de " << inputFormat.getHighestIndex() << " au minimum:\n" << buff << '\n';
			for( auto s: v_str )
				std::cout << "-" << s << '\n';
			throw std::runtime_error("error");
		}
		if( !v_str.empty() )
		if( !v_str[0].empty() )
		{
			auto week_num = getWeekNum(  v_str.at(inputFormat.colIndex[CI_Week])      );
			auto weekday  = getWeekDay(  v_str.at(inputFormat.colIndex[CI_Day])       );
			auto duration = getDuration( v_str.at(inputFormat.colIndex[CI_Duration])  );
			auto name     =              v_str.at(inputFormat.colIndex[CI_Instructor] );
			auto code     =              v_str.at(inputFormat.colIndex[CI_Module]     );

			if( !code.empty() )
			{
//				std::cout << "buff=" << buff << '\n';
//				std::cout << "line=" << line << " nb champs=" << v_str.size() << " name=" << name << " day=" << (int)weekday << " weeknum=" << week_num << " code=" << code << '\n';
				auto type_mod = getTypeModule( code );
				if( v_str.size() > 9 )
					std::cout << "line " << line << ": " << buff << '\n';
				if( name.empty() )
					name = "(néant)";
				results.addOne( name, week_num, weekday, type_mod, duration );
			}
		}
	}
	results.compute();
	auto fn2 = split_string( fn_in, '.' );
	assert( fn2.size() > 0 );

	std::string head1 = "#Nom;Nb jours;Nb sem;vol. CM;vol. TD;vol. TP;vol. total;";
	std::string head2 = ";ratio vol. total/nb jours\n";

	results.writeCsv_I(     "adepopro_E_"  + fn2[0] + ".csv", head1 + "nb modules"     + head2 );
	results.writeCsv_M(     "adepopro_M_"  + fn2[0] + ".csv", head1 + "nb enseignants" + head2 );
	results.writeReport_MI( "adepopro_ME_" + fn2[0] + ".txt", sortBySemester );
	results.writeReport_IM( "adepopro_EM_" + fn2[0] + ".txt" );

}
//-------------------------------------------------------------------

