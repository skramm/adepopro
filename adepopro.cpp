/**
\file adepopro.cpp

- Author: S. Kramm - 2018
- Licence: GPL v3.0
- See https://github.com/skramm/adepopro
*/

#include <vector>
#include <map>
#include <set>
//#include <algorithm>
//#include <functional>
#include <cassert>
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
		_vol[0] = _vol[1] = _vol[2] = 0.0f;
	}
	Triplet( EN_Type ty, float duration )
	{
		_vol[0] = _vol[1] = _vol[2] = 0.0f;
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
};

/// Returns type of course and course code from string
/**
The course code embeds the type: CM/TD/TP, coded as last character.
For example: \c ABC1234D
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
struct InstructorData
{
//	std::string _name;
	size_t      _nbDays = 0;   ///< nb days presence
	Triplet     _volume;       ///< teaching volume
	size_t      _nbModules = 0;

	void incrementDays( size_t n )
	{
		_nbDays += n;
	}
	friend std::ostream& operator << ( std::ostream& f, const InstructorData& ins )
	{
		f << ins._nbDays << g_ocs << ins._volume << g_ocs << ins._nbModules;
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
	file << text << '\n';
	return file;
}
//-------------------------------------------------------------------
struct Results
{
	std::map<std::string,InstructorData> _instructorData; // key: instructor name

	std::map<
		std::string,  // name
		std::map<
			size_t,   // week number
			std::set<EN_WeekDay>
		>
	> _instructorWorkdays; // nb jours de présence par prof

	std::map<
		std::string,      /// module
		std::map<
			std::string,  /// instructor
			Triplet
		>
	> _mod_prof;

	std::map<
		std::string,      /// instructor
		std::map<
			std::string,  /// module
			Triplet
		>
	> _prof_mod;

/// Add one event to the data
	void addOne( std::string name, size_t num_sem, EN_WeekDay wd, const std::pair<EN_Type,std::string>& type_mod, float duration )
	{
		assert( !name.empty() );
		auto& inst = _instructorWorkdays[name];
		auto& week = inst[num_sem];
		week.insert( wd );

		auto type   = type_mod.first;
		auto module = type_mod.second;

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
		for( const auto& prof: _instructorWorkdays )
		{
//			std::cout << "-processing prof " << prof.first << '\n';
			for( const auto& sem: prof.second )
			{
//				std::cout << "  -processing sem " << sem.first << '\n';
//				for( const auto& wd: sem.second )
					_instructorData[prof.first].incrementDays( sem.second.size() );
			}
		}
		for( const auto& pr: _prof_mod )
		{
//			auto prof = pr.first;
			Triplet tot;
			size_t nbMod = 0;
			for( const auto& mod: pr.second )
			{
				tot += mod.second;
				nbMod++;
			}

			_instructorData[pr.first]._volume = tot;
			_instructorData[pr.first]._nbModules = nbMod;
		}
	}

	void writeReport_ME( std::string fn )
	{
		std::cout << " - génération du fichier " << fn << '\n';
		auto file = openFile( fn, "Bilan module/enseignant" );

		char current_semestre = ' ';
		for( const auto& mod: _mod_prof )
		{
			auto module = mod.first;
			auto semestre = module.substr( 4, 1 ).at(0);
			if( current_semestre != semestre )
			{
				file << "\n *** SEMESTRE *** " << semestre << '\n';
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

	void writeReport_EM( std::string fn )
	{
		std::cout << " - génération du fichier " << fn << '\n';
		auto file = openFile( fn, "Bilan enseignant/module" );

		for( const auto& pr: _prof_mod )
		{
			auto prof = pr.first;
			file << "\nEnseignant:" << prof << '\n';
			Triplet tot;
			for( const auto& mod: pr.second )
			{
				file << "  - module: " << mod.first << ": " <<  mod.second << '\n';
				tot += mod.second;
			}
			file << "- TOTAL=" << tot << '\n';
		}
	}

	void writeCsv( std::string fn )
	{
		std::cout << " - génération du fichier " << fn << '\n';
		std::ofstream file( fn );
		if( !file.is_open() )
			throw std::runtime_error( "Error, unable to open file " + fn );

		file << "#Resultats:\n";
		file << "#Nom;Nb-jours;vol-CM;vol-TD;vol-TP;vol total;nb-modules;ratio vol. total/nb jours\n";
		for( const auto& instr: _instructorData )
		{
			auto instrData = instr.second;
			file << instr.first << g_ocs << instrData << g_ocs
				<< 1.0*instrData._volume.sum() / instrData._nbDays
				<< '\n';
		}
	}
};
//-------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	assert( argc == 2 );
	std::string fn_in = argv[1];

	std::ifstream file( fn_in );
	if( !file.is_open() )
		throw std::runtime_error( "Error, unable to open file " + fn_in );

	Results results;

	char delimiter{';'};
	std::string buff;
	int line = 0;
	while ( getline (file, buff ) )
	{
		line++;
//		std::cout << "line " << line << ": " << buff << '\n';

		auto v_str = split_string( buff, delimiter );
//		std::cout << "nb champs=" << v_str.size() << '\n';
		if( v_str.size() < 9 && !v_str.empty() )
		{
			std::cout << "erreur, champ manquants: " << v_str.size() << " au lieu de 9 :\n" << buff << '\n';
			for( auto s: v_str )
				std::cout << "-" << s << '\n';
			throw std::runtime_error("error");
		}
		if( !v_str.empty() )
		if( !v_str[0].empty() )
		{
			auto week_num = getWeekNum( v_str[0] );
			auto weekday  = getWeekDay( v_str[1] );
			auto name = v_str[6];
			auto code = v_str[8];
			auto duration = getDuration( v_str[2] );
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

	results.writeCsv(    "adepopro_" + fn2[0] + ".csv" );
	results.writeReport_ME( "adepopro_ME_" + fn2[0] + ".txt" );
	results.writeReport_EM( "adepopro_EM_" + fn2[0] + ".txt" );

}
//-------------------------------------------------------------------
