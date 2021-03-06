// Copyright (c) 2011, Regents of the University of Utah
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the <organization> nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "smoothingGrid.H"
#include <iostream>
#include <fstream>
#include "marchingTet.H"
#include "Common.h"

#include <float.h>

#include <tbb/tbb.h>


#if defined(_WIN32) || defined(_WIN64)
#  include "getopt.h"
#  include <windows.h>
//#  include <time.h>

int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME system_time;
    FILETIME   file_time;
    uint64_t   time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time  = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}

#else
#  include <getopt.h>
#  include <sys/time.h>
#endif





#if 1
// sph
bool readfile(char* inPosFile, char* inVelFile, std::vector<SlVector3>& particles, std::vector<Real>& radii, std::vector<SlVector3>& velocities)
{
    unsigned int  numPoints;
    unsigned int  numPointsInVelFile;
    std::ifstream inPos(inPosFile, std::ios::in | std::ios::binary);
    std::ifstream inVel(inVelFile, std::ios::in | std::ios::binary);
    inPos.read((char*)&numPoints, sizeof(numPoints));
    float radius;
    inPos.read((char*)&radius, sizeof(float));

    inVel.read((char*)&numPointsInVelFile, sizeof(numPoints));
    if(numPoints != numPointsInVelFile)
    {
        printf("Error: inconsistent number of particles in pos file aned vel file.\n");
        exit(EXIT_FAILURE);
    }

    printf("Num particles: %u\n", numPoints);

    particles.reserve(numPoints);
    velocities.reserve(numPoints);
    radii.reserve(numPoints);

    for(unsigned int i = 0; i < numPoints; i++)
    {
        float x, y, z, xvel, yvel, zvel;

        inPos.read((char*)&x, sizeof(x));
        inPos.read((char*)&y, sizeof(y));
        inPos.read((char*)&z, sizeof(z));

        inVel.read((char*)&xvel, sizeof(x));
        inVel.read((char*)&yvel, sizeof(y));
        inVel.read((char*)&zvel, sizeof(z));


        particles.push_back(SlVector3(x, y, z));
        velocities.push_back(SlVector3(xvel, yvel, zvel));
        radii.push_back(radius);
    }

    return true;
}

#else
#if 0
// sphere
bool readfile(char* infname, std::vector<SlVector3>& particles, std::vector<Real>& radii, std::vector<SlVector3>& velocities)
{
    std::ifstream in(infname, std::ios::in | std::ios::binary);
    SlVector3     p;
    while(!in.eof())
    {
        in >> p[0] >> p[1] >> p[2];
        if(!in.eof()) particles.push_back(p);
    }
    return true;
}

#else
#if 1
// jihun
bool readfile(char* inPosFile, std::vector<SlVector3>& particles, std::vector<Real>& radii, std::vector<SlVector3>& velocities)
{
    std::ifstream in(inPosFile, std::ios::in | std::ios::binary);
    int           numPoints;
    in.read((char*)&numPoints, sizeof(int));
    SlVector3* particlesTemp = new SlVector3[numPoints];
    particles.reserve(numPoints);
    in.read((char*)particlesTemp, sizeof(SlVector3) * numPoints);
    for(int i = 0; i < numPoints; i++)
    {
        particles.push_back(particlesTemp[i]);
    }
    delete [] particlesTemp;
    return true;
}

#else
// fun
bool readfile(char* inPosFile, std::vector<SlVector3>& particles, std::vector<Real>& radii, std::vector<SlVector3>& velocities)
{
    std::ifstream in(inPosFile, std::ios::in | std::ios::binary);
    SlVector3     p;
    Real          junk;
    while(!in.eof())
    {
        in >> p[0] >> p[1] >> p[2] >> junk;
        if(!in.eof()) particles.push_back(p);
    }
    return true;
}

#endif
#endif
#endif

void dumpSurface(char* outfname, const SmoothingGrid& grid)
{
    std::vector<SlTri>     triangles;   // triangles in polygon mesh
    std::vector<SlVector3> meshPts;     // points in polygon mesh
    std::vector<SlVector3> normals;     // normals (at points) in polygon mesh

    MarchingTet mc(grid.nx, grid.ny, grid.nz, grid.h, grid.bbMin);
    mc.buildTriangleMesh(grid.phi, triangles, meshPts);

    std::ofstream                          out(outfname, std::ios::out);
    std::vector<SlVector3>::const_iterator p;
    std::vector<SlTri>::const_iterator     t;
    std::vector<SlVector3>::const_iterator n;
    //std::cout<<triangles.size()<<" "<<meshPts.size()<<std::endl;

    for(p = meshPts.begin(); p != meshPts.end(); p++)
        out << "v " << (*p)[0] << " " << (*p)[1] << " " << (*p)[2] << std::endl;
    //for (n=normals.begin(); n!=normals.end(); n++)
    //	out<<"vn "<<(*n)[0]<<" "<<(*n)[1]<<" "<<(*n)[2]<<std::endl;
    for(t = triangles.begin(); t != triangles.end(); t++)
        out << "f " << (*t)[0] + 1 << " " << (*t)[1] + 1 << " " << (*t)[2] + 1 << std::endl;

    //std::cout <<"output file:"<< outfname<<" done."<<std::endl;
}

int main(int argc, char** argv)
{
    unsigned int flags = 0;
    static int   verboseFlag = 0;
    bool         helpFlag = false;
    int          iterLaplace = 15, iterBiharmonic = 500, redistanceFrequency = 50;
    Real         rmin = -REAL_MAX, rmax = -REAL_MAX, rinit = -REAL_MAX, velGain = 1.0,
                 dtLaplace = -REAL_MAX, dtBiharmonic = -REAL_MAX, dtBiharmonicGain = 1.0;
    Real maxStretch = 4, rratio = 4;



    static struct option long_options[] = {
        { "help", no_argument, 0, 'h' },
        { "verbose", no_argument, &verboseFlag, 1 },
        { "lapiter", required_argument, 0, 'l' },
        { "bihiter", required_argument, 0, 'b' },
        { "laptime", required_argument, 0, 't' },
        { "bihtime", required_argument, 0, 'T' },
        { "rmax", required_argument, 0, 'M' },
        { "rmin", required_argument, 0, 'm' },
        { "redist", required_argument, 0, 'f' },
        { "variable_radius", no_argument, 0, 'V' },
        { "neighbor_anisotropy", no_argument, 0, 'n' },
        { "velocity_anisotropy", required_argument, 0, 'v' },
        { "timestepConst", required_argument, 0, 'B' },
        { "maxStretch", required_argument, 0, 's' },
        { "rratio", required_argument, 0, 'r' },
        { "rinit", required_argument, 0, 'i' },
        { 0, 0, 0, 0 }
    };



    while(1)
    {
        int option_index = 0;
        int c            = getopt_long(argc, argv, "hl:b:t:T:M:m:f:Vnv:B:s:r:g:i:",
                                       long_options, &option_index);
        if(c == -1) break;
        switch(c)
        {
            case 0:
                if(long_options[option_index].flag != 0)
                    break;
                break;

            case 'h':
                helpFlag = true;
                break;

            case 'V':
                flags |= SmoothingGrid::VARIABLE_RADIUS;
                break;

            case 'n':
                flags |= SmoothingGrid::NEIGHBOR_ANISOTROPY;
                break;

            case 'v':
                flags  |= SmoothingGrid::VELOCITY_ANISOTROPY;
                velGain = atof(optarg);
                break;

            case 't':
                dtLaplace = atof(optarg);
                break;

            case 'T':
                dtBiharmonic = atof(optarg);
                break;

            case 'M':
                rmax = atof(optarg);
                break;

            case 'm':
                rmin = atof(optarg);
                break;

            case 'f':
                redistanceFrequency = atoi(optarg);
                break;

            case 'l':
                iterLaplace = atoi(optarg);
                break;

            case 'b':
                iterBiharmonic = atoi(optarg);
                break;

            case 'r':
                rratio = atof(optarg);
                break;

            case 'B':
                dtBiharmonicGain = atof(optarg);
                break;

            case 's':
                maxStretch = atof(optarg);
                break;

            case 'i':
                rinit = atof(optarg);
                break;

            case '?':
                break;

            default:
                abort();
        }
    }

    verboseFlag = 1;
    if(verboseFlag) flags |= SmoothingGrid::VERBOSE;

    if(helpFlag)
    {
        std::cout << "Welcome! " << std::endl;
        std::cout << "Usage: " << argv[0] << " grid_spacing inputFile outputFile" << std::endl;
        std::cout << "Available switches are......." << std::endl;
        std::cout << "-h/--help -> display this message " << std::endl;
        std::cout << "--verbose -> output information in command line " << std::endl;
        std::cout << "-r/--rratio number -> r_max =  number * r_min, Default value is 4\n\t(over-riden by -M)" << std::endl;
        std::cout << "-i/--rinit number -> r_init = number * r_min, otherwise\n\tr_init = 0.5 * (r_min + r_max) " << std::endl;

        std::cout << "-V/--variable_radius min max -> The particles have variable radiuses,\n\tin this case r_min, r_max, and r_init are multipliers for the individual particle radii, you will want to use -m and -M with this option" << std::endl;
        std::cout << "-n/--neighbor_anisotropy -> Turn on neighborhood based anisotropy " << std::endl;
        std::cout << "-v/--velocity_anisotropy number -> Turn on velocity-based anisotropy,\n\tthe number is a gain on the amount of anisotropy,\n\tlarger values lead to more anisotropy " << std::endl;
        std::cout << "-s/--maxStretch number -> maximum amount of anisotropy (condition number of G),\n\tDefault value is 4 " << std::endl;

        std::cout << "-l/--lapiter number -> Number of laplacian smoothing passes,\n\tDefault value is 15" << std::endl;
        std::cout << "-b/--bihiter number -> Number of biharmonic smoothing passes,\n\tDefault value is 500" << std::endl;
        std::cout << "-B/--timestepConst number -> A multiplier for the biharmonic timestep " << std::endl;
        std::cout << "-t/--laptime number -> Timestep for laplacian smoothing\n\t Deafult value is 0.1*h^2" << std::endl;
        std::cout << "-T/--bihtime number -> Timestep for biharmonic smoothing,\n\tDefault value is 0.01*h^4" << std::endl;
        std::cout << "-r/--redist number -> Frequency of redistancing, Default value is 50" << std::endl;

        std::cout << "-m/--rmin number -> Minimum radius,\n\tDefault value is (0.5 * sqrt(3) * grid_spacing)" << std::endl;
        std::cout << "-M/--rmax number -> Maximum radius, Default value is (4 * rmin)" << std::endl;

        exit(1);
    }

    Real  h         = atof(argv[optind++]);
    char* inPosFile = argv[optind++];
    char* inVelFile = argv[optind++];
    char* outfname  = argv[optind++];

    if(rmin == -REAL_MAX)
    {
        rmin = 0.86603 * h;         // 0.5*sqrt(3)*h
        // there could be a stretch up to this amount, we want to make sure that particles
        // all touch at least one grid point, so make the radius bigger...
        if(flags & SmoothingGrid::NEIGHBOR_ANISOTROPY)
        {
            rmin *= sqrt(maxStretch);
        }
        else if(flags & SmoothingGrid::VELOCITY_ANISOTROPY)
        {
            rmin *= cbrt(maxStretch);
        }
    }
    if(rmax == -REAL_MAX) rmax = rratio * rmin;
    if(rinit == -REAL_MAX) rinit = 0.5 * (rmin + rmax);
    else if(!(flags & SmoothingGrid::VARIABLE_RADIUS))
        rinit *= rmin;
    if(dtLaplace == -REAL_MAX) dtLaplace = 0.1 * h * h;
    if(dtBiharmonic == -REAL_MAX) dtBiharmonic = 0.01 * dtBiharmonicGain * h * h * h * h;

    timeval startTime, endTime, beginTime;
    gettimeofday(&beginTime, NULL);

    std::vector<SlVector3> particles, velocities;
    std::vector<Real>      radii;
    gettimeofday(&startTime, NULL);
    readfile(inPosFile, inVelFile, particles, radii, velocities);
    gettimeofday(&endTime, NULL);
    if(verboseFlag)
        std::cout << "Reading the file took " << (endTime.tv_sec - startTime.tv_sec) +
        (endTime.tv_usec - startTime.tv_usec) * 1.0e-6 << std::endl;






    tbb::task_scheduler_init threadInit(tbb::task_scheduler_init::automatic);
    (void)threadInit;









    gettimeofday(&startTime, NULL);
    SmoothingGrid grid(h, rmin, rmax, rinit, velGain, maxStretch, flags, particles, radii, velocities);
    gettimeofday(&endTime, NULL);
    if(verboseFlag)
        std::cout << "Initialization took " << (endTime.tv_sec - startTime.tv_sec) +
        (endTime.tv_usec - startTime.tv_usec) * 1.0e-6 << std::endl;

    gettimeofday(&startTime, NULL);
    grid.doLaplacianSmoothing(iterLaplace, dtLaplace, redistanceFrequency);
    gettimeofday(&endTime, NULL);
    if(verboseFlag)
        std::cout << "Laplacian Smoothing took " << (endTime.tv_sec - startTime.tv_sec) +
        (endTime.tv_usec - startTime.tv_usec) * 1.0e-6 << std::endl;

    gettimeofday(&startTime, NULL);
    grid.doBiharmonicSmoothing(iterBiharmonic, dtBiharmonic, redistanceFrequency);
    gettimeofday(&endTime, NULL);
    if(verboseFlag)
        std::cout << "Biharmonic Smoothing took " << (endTime.tv_sec - startTime.tv_sec) +
        (endTime.tv_usec - startTime.tv_usec) * 1.0e-6 << std::endl;

    gettimeofday(&startTime, NULL);
    dumpSurface(outfname, grid);
    gettimeofday(&endTime, NULL);
    if(verboseFlag)
        std::cout << "Dumping file with marching tet took " << (endTime.tv_sec - startTime.tv_sec) +
        (endTime.tv_usec - startTime.tv_usec) * 1.0e-6 << std::endl;


    std::cout << "Total: " << (endTime.tv_sec - beginTime.tv_sec) +
    (endTime.tv_usec - beginTime.tv_usec) * 1.0e-6 << std::endl;



    return 0;
}
