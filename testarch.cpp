
#include <fstream>
#include "Bars.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;

int main()
{
    std::string filename = "./test";

    std::ofstream out(filename);

    boost::archive::text_oarchive text_output_archive(out);

    Bars* Bars = new ::Bars();

    text_output_archive & (Bars);

    Bars->Delete();

    out.close();

    return 0;
}
