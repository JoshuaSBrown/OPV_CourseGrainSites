#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <string>
#include <random>
#include <map>
#include <unordered_map>
#include <set>
#include <cmath>
#include <cassert>
#include <algorithm>

#include "../../../include/kmccoarsegrain/kmc_coarsegrainsystem.hpp"
#include "../../../include/kmccoarsegrain/kmc_walker.hpp"

using namespace std;
using namespace std::chrono;
using namespace kmccoarsegrain;

bool sortbysec(const pair<int,double> &a,
              const pair<int,double> &b)
{
    return (a.second < b.second);
}
/**
 * \brief class for converting 1d array to 3d and vice versa
 **/
class Converter {
  public: 
    Converter(int distance) { distance_=distance;}
    int to1D(int x,int y, int z){
      assert(x<distance_);
      assert(y<distance_);
      assert(z<distance_);
      return ( z* distance_ * distance_ ) + (y *distance_) + x;
    }
    int to1D(vector<int> position){
      return ( position.at(2)* distance_ * distance_ ) + (position.at(1) *distance_) + position.at(0);
    }
    vector<int> to3D(int index) {
      vector<int> position(3,0);
      position.at(2) = index / (distance_*distance_);
      index-=(position.at(2) *distance_*distance_);
      position.at(1) = index/distance_;
      position.at(0) = index % distance_;
      return position;
    }
  private:
    int distance_;
};

bool compareSecondItemOfPair(const pair<int,double> &x, const pair<int,double> & y){
  return x.second<y.second;
}

int main(int argc, char* argv[]){

  if(argc!=9){
    cerr << "To run the program correctly you must provide the " << endl;
    cerr << "following parameters: " << endl;
    cerr << endl;
    cerr << "sigma        - defines the width of the density of states it" << endl;
    cerr << "               must be a double. " << endl;
    cerr << "distance     - integer defines the width, length and height" << endl;
    cerr << "               of the simulation box in terms of the number" << endl;
    cerr << "               of sites. Distance between sites is set to 1" << endl;
		cerr << "               nm." << endl;
    cerr << "seed         - integer value defines the random number seed" << endl;
    cerr << "walkers      - integer value defines number of walkers." << endl;
    cerr << "threshold    - integer value defines minimum threshold of " << endl;
    cerr << "               how often the simulation will try to coarse " << endl;
    cerr << "               grain." << endl;
    cerr << "sample       - a double defining how often metrics will be" << endl;
    cerr << "rate           measured." << endl;
		cerr << "field        - a double defining the electric field across" << endl;
		cerr << "               the x dimension of the system, in units of " << endl;
		cerr << "               V/nm" << endl;
		cerr << "correlation  - The characteristic strength of the correlation" << endl;
	  cerr << "radius" << endl;
    cerr << endl;
    cerr << "To run:" << endl;
    cerr << endl;
    cerr << "./performance_test_crude_vs_coarsegraining sigma distance seed walkers threshold sampe_rate electric_field" << endl;
    cerr << endl;
    return -1;
  }

  double sigma  = stod(string(argv[1]));
  int distance  = stoi(string(argv[2]));
  int seed      = stoi(string(argv[3]));
  int walkers   = stoi(string(argv[4]));
  int threshold = stoi(string(argv[5]));
  double sample_rate = stod(string(argv[6]));
	double electric_field = stod(string(argv[7]));
	double correlation_radius = stod(string(argv[8]));
	// Corilation specific parameters
	double percentage_coorilation_seeds = 0.001;

	// The default was 3.0
	//double correlation_radius = 3.0;
	double max_correlation = correlation_radius*1.5;
	int inverse_percentage = static_cast<int>(1.0/percentage_coorilation_seeds);
	int num_seeds = distance*distance*distance/inverse_percentage; 

  cout << endl;
  cout << "Parameters passed in:" << endl;
  cout << endl;
  cout << "sigma:                       " << sigma << endl;
  cout << "distance:                    " << distance << endl;
  cout << "seed:                        " << seed << endl;
  cout << "walkers:                     " << walkers << endl;
  cout << "threshold:                   " << threshold << endl;
  cout << "sample rate:                 " << sample_rate << endl;
  cout << "electric_field:              " << electric_field << endl;
	cout << "coorelation radius:          " << correlation_radius << endl;
	cout << "number of coorelation seeds: " << num_seeds << endl;
  cout << endl;

  /// Create Energies and place them in a vector
  cout << "Filling sites with energies from a guassian distribution " << endl;
  cout << "centered at 0.0." << endl;
  cout << "sigma of " << sigma << endl;
  cout << endl;
  cout << endl;

  // Record setup time
  high_resolution_clock::time_point setup_time_start = high_resolution_clock::now();
  vector<double> energies;
	double min_energy;
	double max_energy;
	int totalNumberSites = distance*distance*distance;
  int number_of_bins = 20;
	double pre_correlation_fwhm;
	double post_correlation_fwhm;
	{
		cout << "Pre correlation energy assignment" << endl;
		{ // Precorrelation energy assignment
			mt19937 random_number_generator;
			random_number_generator.seed(seed);
			normal_distribution<double> distribution(0.0,sigma);

			int totalNumberSites = distance*distance*distance;

			energies.push_back(distribution(random_number_generator));
			min_energy = energies.at(0);
			max_energy = energies.at(0);
			for(int i=1;i<totalNumberSites;++i){
				energies.push_back(distribution(random_number_generator));
				if(min_energy> energies.at(i)) min_energy = energies.at(i);
				if(max_energy<energies.at(i)) max_energy = energies.at(i);
			}
		}

		cout << "Pre correlation min energy " << min_energy << " max energy " << max_energy << endl;
		{ // Determine full width half max

			 vector<double> bins(number_of_bins,0.0);
      double bin_increment = (max_energy-min_energy)/static_cast<double>(number_of_bins);
      for( auto energy : energies){
        double bin_edge = min_energy;
        for( int i=0; i<number_of_bins;i++){
          bin_edge+=bin_increment;
          if(energy<=bin_edge){
            bins.at(i)++;
            break;
          }
        }
      }

      int max_count_in_bin = 0;
      for(auto count : bins){
        if(count>max_count_in_bin) max_count_in_bin=count;
      }

			    // Determine halfway point
      double half_way_count = static_cast<double>(max_count_in_bin)/static_cast<double>(number_of_bins);

      double lower_intersect;
      {

        double lower_min_count = 0.0;
        double lower_min_half = 0.0;
        double upper_min_count = 0.0;
        double upper_min_half = 0.0;

        double previous_count = 0.0;

        double bin_edge = min_energy;
        for(auto count : bins){

          double count_double = static_cast<double>(count);
          if(count_double>half_way_count){
            upper_min_count = count_double;
            upper_min_half = (bin_edge+bin_increment)/2.0;
            lower_min_count = previous_count;
            lower_min_half = upper_min_half-bin_increment;
            break;
          }
          bin_edge+=bin_increment;
          previous_count = count_double;
        }

        auto slope = (upper_min_count-lower_min_count)/(upper_min_half-lower_min_half);
        auto constant = lower_min_count-slope*lower_min_half;
        lower_intersect = (half_way_count-constant)/slope;
			}

			double upper_intersect;                                                     
			{                                                                           

				double lower_max_count = 0.0;                                             
				double lower_max_half = 0.0;                                              
				double upper_max_count = 0.0;                                             
				double upper_max_half = 0.0;                                              

				double previous_count = 0.0;                                              

				double bin_edge = max_energy;                                             
				for(int index=(number_of_bins-1);index>=0;index--){                       

					int count = bins.at(index);                                             
					double count_double = static_cast<double>(count);                       
					if(count_double>half_way_count){                                        
						lower_max_count = count_double;                                       
						lower_max_half = (bin_edge-bin_increment)/2.0;                        
						upper_max_count = previous_count;                                     
						upper_max_half = upper_max_half+bin_increment;                        
						break;                                                                
					}                                                                       
					bin_edge-=bin_increment;                                                
					previous_count = count_double;                                          
				}                                                                         

				auto slope = (upper_max_count-lower_max_count)/(upper_max_half-lower_max_half);
				auto constant = lower_max_count-slope*lower_max_half;                     
				upper_intersect = (half_way_count-constant)/slope;                        
			}                                                                           

			pre_correlation_fwhm = upper_intersect-lower_intersect;               
			cout << "Pre correlation fwhm " << pre_correlation_fwhm << endl;  
			assert(pre_correlation_fwhm > 0.0 && "FWHM must be a positive number");
		} // Determine full width half max
		
  }
  Converter converter(distance);

	{ // Correlate Energies

		mt19937 random_number_generator;                                            
		random_number_generator.seed(seed+7);                                            
		uniform_int_distribution<int> distribution(0,distance*distance*distance-1); 

		vector<int> seeds;                                                          
		for(int seed_index = 0 ;seed_index< num_seeds;++seed_index){                
			seeds.push_back(distribution(random_number_generator));                   
		}                                                                           

		unordered_map<int,double> sites_distance_to_seed;
		double distance_double = static_cast<double>(distance);
		for( auto seed : seeds ){
			energies.at(seed) = 0.0;
			auto position = converter.to3D(seed);
			double x_pos = static_cast<double>(position.at(0));
			double y_pos = static_cast<double>(position.at(1));
			double z_pos = static_cast<double>(position.at(2));

			for(double x_neigh = x_pos - max_correlation; x_neigh< x_pos+max_correlation; x_neigh+=1.0  ){
				for(double y_neigh = y_pos - max_correlation; y_neigh< y_pos+max_correlation; y_neigh+=1.0){
					for(double z_neigh = z_pos - max_correlation; z_neigh< z_pos+max_correlation; z_neigh+=1.0){
						if(x_neigh<distance_double && x_neigh>-0.0){
							if(y_neigh<distance_double && y_neigh>-0.0){
								if(z_neigh<distance_double && z_neigh>-0.0){
									auto xdiff = pow(x_pos-x_neigh,2.0);
									auto ydiff = pow(y_pos-y_neigh,2.0);
									auto zdiff = pow(z_pos-z_neigh,2.0);
									double neigh_radius = pow(xdiff+ydiff+zdiff,1.0/2.0);
									int x_n = static_cast<int>(x_neigh);
									int y_n = static_cast<int>(y_neigh);
									int z_n = static_cast<int>(z_neigh);
									int neighId = converter.to1D(x_n,y_n,z_n);
									assert(neighId<distance*distance*distance);
									// Site has no recorded neighbors
									if(sites_distance_to_seed.count(neighId)==0){
										double energyDiff = energies.at(seed)-energies.at(neighId);
										energies.at(neighId) = energies.at(neighId)+energyDiff*exp(-neigh_radius/correlation_radius);
										sites_distance_to_seed[neighId] = neigh_radius;
									}else if(sites_distance_to_seed[neighId]>neigh_radius){
										// sites current closest seed is futher away 
										double energyDiff = energies.at(seed)-energies.at(neighId);
										energies.at(neighId) = energies.at(neighId)+energyDiff*exp(-neigh_radius/correlation_radius);
										sites_distance_to_seed[neighId] = neigh_radius;
									}
								}
							}
						}
					}
				}
			}
		}

	} // Correlate Energies

		
	{ // Calculate Post correlation FWHM

		 // Calulate the min and max energies fresh
    {
      min_energy = energies.at(0);
      max_energy = energies.at(0);
      for(int i=1;i<totalNumberSites;++i){
        double energy = energies.at(i);
        if(energy<min_energy) min_energy = energy;
        if(energy>max_energy) max_energy = energy;
      }
    }
		cout << "Post correlation min energy " << min_energy << " max energy " << max_energy << endl;
		// Determine the full width half maximum
		{
			vector<double> bins(number_of_bins,0.0);
			double bin_increment = (max_energy-min_energy)/static_cast<double>(number_of_bins);
			for( auto energy : energies){
				double bin_edge = min_energy;
				for( int i=0; i<number_of_bins;i++){
					bin_edge+=bin_increment;
					if(energy<=bin_edge){
						bins.at(i)++;
						break;
					}
				}
			}

			int max_count_in_bin = 0;
			for(auto count : bins){
				if(count>max_count_in_bin) max_count_in_bin=count;
			}


			// Determine halfway point                                                  
			double half_way_count = static_cast<double>(max_count_in_bin)/static_cast<double>(number_of_bins);

			double lower_intersect;                                                     
			{                                                                           

				double lower_min_count = 0.0;                                             
				double lower_min_half = 0.0;                                              
				double upper_min_count = 0.0;                                             
				double upper_min_half = 0.0;                                              

				double previous_count = 0.0;                                              

				double bin_edge = min_energy;                                             
				for(auto count : bins){                                                   

					double count_double = static_cast<double>(count);                       
					if(count_double>half_way_count){                                        
						upper_min_count = count_double;                                       
						upper_min_half = (bin_edge+bin_increment)/2.0;                        
						lower_min_count = previous_count;                                     
						lower_min_half = upper_min_half-bin_increment;                        
						break;                                                                
					}                                                                       
					bin_edge+=bin_increment;                                                
					previous_count = count_double;                                          
				}                                                                         

				auto slope = (upper_min_count-lower_min_count)/(upper_min_half-lower_min_half);
				auto constant = lower_min_count-slope*lower_min_half;                     
				lower_intersect = (half_way_count-constant)/slope;                        
			}         


			double upper_intersect;
			{

				double lower_max_count = 0.0;
				double lower_max_half = 0.0;
				double upper_max_count = 0.0;
				double upper_max_half = 0.0;

				double previous_count = 0.0;

				double bin_edge = max_energy;
				for(int index=(number_of_bins-1);index>=0;index--){

					int count = bins.at(index);
					double count_double = static_cast<double>(count);
					if(count_double>half_way_count){
						lower_max_count = count_double;
						lower_max_half = (bin_edge-bin_increment)/2.0;
						upper_max_count = previous_count;
						upper_max_half = upper_max_half+bin_increment;
						break;
					}
					bin_edge-=bin_increment;
					previous_count = count_double;
				}

				auto slope = (upper_max_count-lower_max_count)/(upper_max_half-lower_max_half);
				auto constant = lower_max_count-slope*lower_max_half;
				upper_intersect = (half_way_count-constant)/slope;
			}

			post_correlation_fwhm = upper_intersect-lower_intersect;
			cout << "Post correlation fwhm " << post_correlation_fwhm << endl;            
		} // Calculate Post Correlation FWHM

		{ // Scaling energies
			double scale = pre_correlation_fwhm/post_correlation_fwhm;
			cout << "Scaling by " << scale << endl;
			for(size_t ind = 0; ind< energies.size();++ind){
				energies.at(ind)*=scale;
			}
		
		}
	} // ENd of correlation

  unordered_map<int,unordered_map<int,double>> rates;
  unordered_map<int,vector<int>> neighbors;
  {

    double reorganization_energy = 0.01;
    double J = 0.01;
    double kBT = 0.025;
    cout << "Calculating rates using Semiclassical Marcus theory assuming: " << endl;
    cout << endl;
    cout << "reoganization energy lambda:          " << reorganization_energy << endl;
    cout << "transfer integral J:                  " << J << endl;
    cout << "Boltzmann constant * temperature kBT: " << kBT << endl;

    double hbar = pow(6.582,-16);
    double pi = 3.14;

    // Define marcus coefficient
    double coef = 2*pi/hbar*pow(J,2.0)*1/pow(4*pi*kBT,1.0/2.0);


    for(int x=0; x<(distance-1); ++x){
      for(int y=0;y<(distance-1);++y){
        for(int z=0;z<(distance-1);++z){
          // Define neighbors
          int xlow = x;
          int xhigh = x+1;

          int ylow = y;
          int yhigh = y+1;
          
          int zlow = z;
          int zhigh = z+1;

         /* if(xhigh+2<distance) ++xhigh;
          if(yhigh+2<distance) ++yhigh;
          if(zhigh+2<distance) ++zhigh;*/

					int reverse_siteId;// = neighId;
          int siteId = converter.to1D(x,y,z); 
          for( int x2 = xlow; x2<=xhigh; ++x2){
            for( int y2 = ylow; y2<=yhigh; ++y2){
              for( int z2 = zlow; z2<=zhigh; ++z2){

								// eV change in energy of down field hop vs up field
								double energy_diff = electric_field*(x-x2);
                assert(x2>=0);
                assert(x2<distance);
                assert(y2>=0);
                assert(y2<distance);
                assert(z2>=0);
                assert(z2<distance);
                int neighId = converter.to1D(x2,y2,z2);
                if(siteId!=neighId){
                  neighbors[siteId].push_back(neighId);
                  double deltaE = energies.at(neighId)-energies.at(siteId)+energy_diff;
                  double exponent = -pow(reorganization_energy-deltaE,2.0)/(4.0*reorganization_energy*kBT);
                  rates[siteId][neighId] = coef*exp(exponent);
                }

								energy_diff = electric_field*(x2-x);
                assert(x>=0);
                assert(x<distance);
                assert(y>=0);
                assert(y<distance);
                assert(z>=0);
                assert(z<distance);
								reverse_siteId = neighId;
                neighId = siteId;
                if(reverse_siteId!=neighId){
                  neighbors[reverse_siteId].push_back(neighId);
                  double deltaE = energies.at(neighId)-energies.at(reverse_siteId)+energy_diff;
                  double exponent = -pow(reorganization_energy-deltaE,2.0)/(4.0*reorganization_energy*kBT);
                  rates[reverse_siteId][neighId] = coef*exp(exponent);
                }

              }
            }
          }
          assert(rates[siteId].size()!=0);          
          assert(rates[reverse_siteId].size()!=0);          
        }
      }
    }
  }

  // Place walkers randomly in the first plane of the system
  set<int> siteOccupied;
  unordered_map<int,vector<int>> walker_positions;
  {
    mt19937 random_number_generator;
    random_number_generator.seed(seed+1);
    uniform_int_distribution<int> distribution(0,distance-1);
    int walker_index = 0;
    while(walker_index<walkers){
      int x = 0;
      int y = distribution(random_number_generator);
      int z = distribution(random_number_generator);
      assert(x<distance);
      assert(y<distance);
      assert(z<distance);
      assert(x>=0);
      assert(y>=0);
      assert(z>=0);
      int siteId = converter.to1D(x,y,z);
      if(siteOccupied.count(siteId)==0){
        vector<int> position = { x, y, z};
        walker_positions[walker_index] = position;
        ++walker_index;
        siteOccupied.insert(siteId);
      }
    }
  } // Place walkers randomly in the first plane of the system 
  high_resolution_clock::time_point setup_time_end = high_resolution_clock::now();

  cout << "Running crude Monte Carlo" << endl;
  // Crude Monte Carlo
  high_resolution_clock::time_point crude_time_start = high_resolution_clock::now();
  {

    unordered_map<int,double> sojourn_times;
    unordered_map<int,double> sum_rates;
    // Calculate sojourn times & sum_rates
    {
      for(int x=0; x<distance; ++x){
        for(int y=0;y<distance;++y){
          for(int z=0;z<distance;++z){
            // Define neighbors
/*            int xlow = x;
            int xhigh = x;

            int ylow = y;
            int yhigh = y;

            int zlow = z;
            int zhigh = z;

            if(xlow-1>0) --xlow;
            if(xhigh+1<distance) ++xhigh;
            if(ylow-1>0) --ylow;
            if(yhigh+1<distance) ++yhigh;
            if(zlow-1>0) --zlow;
            if(zhigh+1<distance) ++zhigh;
*/
            double sum_rate = 0.0;
            int siteId = converter.to1D(x,y,z); 
						for( const pair<int,double> & neigh_and_rate : rates[siteId]){
 /*           for( int x2 = xlow; x2<=xhigh; ++x2){
              for( int y2 = ylow; y2<=yhigh; ++y2){
                for( int z2 = zlow; z2<=zhigh; ++z2){*/
							//int neighId = neigh_and_rate.first;
               //   int neighId = converter.to1D(x2,y2,z2);
                //  if(siteId!=neighId){
                    sum_rate += neigh_and_rate.second;//rates[siteId][neighId];
                 // }
/*                }
              }
            }*/
						}
            sojourn_times[siteId] = 1.0/sum_rate;        
            sum_rates[siteId] = sum_rate;  
          }
        }
      }
    }// Calculate sojourn times & sum_rates

    unordered_map<int,vector<pair<int,double>>> cummulitive_probability_to_neighbors;
    // Calculate crude probability to neighbors
    {
      for(int x=0; x<(distance); ++x){
        for(int y=0;y<(distance);++y){
          for(int z=0;z<(distance);++z){
            // Define neighbors
/*            int xlow = x;
            int xhigh = x+1;

            int ylow = y;
            int yhigh = y+1;

            int zlow = z;
            int zhigh = z+1;
*/
						int siteId = converter.to1D(x,y,z); 
            vector<pair<int,double>> probability;
 /*           for( int x2 = xlow; x2<=xhigh; ++x2){
              for( int y2 = ylow; y2<=yhigh; ++y2){
                for( int z2 = zlow; z2<=zhigh; ++z2){*/
						for( const pair<int,double> & neigh_rate : rates[siteId]){
                  int neighId = neigh_rate.first;//converter.to1D(x2,y2,z2);
//                  if(siteId!=neighId){
                    probability.push_back(pair<int,double>(neighId,neigh_rate.second/sum_rates[siteId]));
                  }
               // }
              //}
           // }

						sort(probability.begin(),probability.end(),sortbysec);
            vector<pair<int,double>> cummulitive_probability;
						double pval = 0.0;
            double value = 0.0;
						for( pair<int,double> prob : probability ){
							prob.second+=pval;
							pval = prob.second;
            	cummulitive_probability.push_back(pair<int,double>(prob.first,value));
              value = prob.second;
						}
            cummulitive_probability_to_neighbors[siteId] = cummulitive_probability; 
            assert(cummulitive_probability_to_neighbors[siteId].size()!=0);
          }
        }
      }
    }// Calculate crude probability to neighbors


    // Calculate Walker dwell times and sort 
    list<pair<int,double>> walker_global_times;
    {

      mt19937 random_number_generator;
      random_number_generator.seed(seed+2);
      uniform_real_distribution<double> distribution(0.0,1.0);

      for(int walker_index=0; walker_index<walkers;++walker_index){
        auto position = walker_positions[walker_index];
        auto siteId = converter.to1D(position);
        walker_global_times.push_back(pair<int,double>(walker_index, sojourn_times[siteId]*log(distribution(random_number_generator))*-1.0));
      }
      walker_global_times.sort(compareSecondItemOfPair);
    }// Calculate walker dwell times and sort

    // Run simulation until cutoff simulation time is reached
    {

      mt19937 random_number_generator;
      random_number_generator.seed(seed);
      uniform_real_distribution<double> distribution(0.0,1.0);
      unordered_map<int,int> frequency;

      while(!walker_global_times.empty()){
        int walkerId = walker_global_times.begin()->first;
        vector<int> walker_position = walker_positions[walkerId];
        int siteId = converter.to1D(walker_position);
        double random_number = distribution(random_number_generator);
        // Attempt to hop
        assert(cummulitive_probability_to_neighbors[siteId].size()!=0);
				assert(cummulitive_probability_to_neighbors[siteId].size()!=0);
        for(auto it = cummulitive_probability_to_neighbors[siteId].rbegin();
            it!=cummulitive_probability_to_neighbors[siteId].rend();
            ++it){
        if(random_number > it->second){

            int neighId = it->first;
            if(siteOccupied.count(neighId)){
              // Update the sojourn time walker is unable to make the jump
              walker_global_times.begin()->second += sojourn_times[siteId]*log(distribution(random_number_generator))*-1.0;
            }else{
              // vacate site
              siteOccupied.erase(siteId); 
              // Occupy new site
              siteOccupied.insert(neighId);
              if(frequency.count(neighId)){
                ++frequency[neighId];
              }else{
                frequency[neighId]=1;
              }
              // Update the walkers position
              walker_positions[walkerId] = converter.to3D(neighId);
							if(walker_positions[walkerId].at(0)==distance-1){
								walker_global_times.pop_front(); 
								siteOccupied.erase(neighId);
							}else{
								// Update the sojourn time of the walker
								walker_global_times.begin()->second += sojourn_times[neighId]*log(distribution(random_number_generator))*-1.0;
							}
            }
            break;
          }
        }
        // reorder the walkers based on which one will move next
        walker_global_times.sort(compareSecondItemOfPair);
      }
  
    } // Run simulation until cutoff simulation time is reached

    
  } // Crude Mone Carlo
  high_resolution_clock::time_point crude_time_end = high_resolution_clock::now();

	// Reset walker positions
	// Place walkers randomly in the first plane of the system
  siteOccupied.clear();
  walker_positions.clear();
  {
    mt19937 random_number_generator;
    random_number_generator.seed(seed+1);
    uniform_int_distribution<int> distribution(0,distance-1);
    int walker_index = 0;
    while(walker_index<walkers){
      int x = 0;
      int y = distribution(random_number_generator);
      int z = distribution(random_number_generator);
      assert(x<distance);
      assert(y<distance);
      assert(z<distance);
      assert(x>=0);
      assert(y>=0);
      assert(z>=0);
      int siteId = converter.to1D(x,y,z);
      if(siteOccupied.count(siteId)==0){
        vector<int> position = { x, y, z};
        walker_positions[walker_index] = position;
        ++walker_index;
        siteOccupied.insert(siteId);
      }
    }
  } // Place walkers randomly in the first plane of the system 

  // Run coarse grained Monte Carlo
  cout << "Running coarse grained Monte Carlo" << endl;
  high_resolution_clock::time_point coarse_time_start = high_resolution_clock::now();
  {

    class Electron : public KMC_Walker {};
    // Create the electrons using the KMC_Walker class
    vector<pair<int,KMC_Walker>> electrons;        
    {
      for(int walker_index = 0; walker_index<walkers; ++walker_index){
        Electron electron;
        int siteId = converter.to1D(walker_positions[walker_index]);
        electron.occupySite(siteId);
        electrons.push_back(pair<int,KMC_Walker>(walker_index,electron));
      }
    }
    
    // Run the coarse grain simulation
    {
      KMC_CoarseGrainSystem CGsystem;
			//CGsystem.setPerformanceRatio(1.6);
      CGsystem.setRandomSeed(seed);
      CGsystem.setMinCoarseGrainIterationThreshold(threshold);
      CGsystem.setTimeResolution(sample_rate);
      CGsystem.initializeSystem(rates);
      CGsystem.initializeWalkers(electrons);
      // Calculate Walker dwell times and sort 
      list<pair<int,double>> walker_global_times;
      {

        mt19937 random_number_generator;
        random_number_generator.seed(3);
        uniform_real_distribution<double> distribution(0.0,1.0);

        for(int walker_index=0; walker_index<walkers;++walker_index){
          walker_global_times.push_back(pair<int,double>(walker_index,electrons.at(walker_index).second.getDwellTime()));
        }
        walker_global_times.sort(compareSecondItemOfPair);
      }// Calculate walker dwell times and sort
      while(!walker_global_times.empty()){
        auto walker_index = walker_global_times.begin()->first;
        KMC_Walker& electron = electrons.at(walker_index).second; 
        int electron_id = electrons.at(walker_index).first; 
        CGsystem.hop(electron_id,electron);
        // Update the dwell time
        walker_global_times.begin()->second += electron.getDwellTime();
			
        int current_site = electron.getIdOfSiteCurrentlyOccupying();
				auto position = converter.to3D(current_site);
        //cout << "walker " << electron_id << " Current site " << current_site << " pos " << position.at(0) << " " << position.at(1) << " " << position.at(2) << endl;

        if(position.at(0)==distance-1){
					CGsystem.removeWalkerFromSystem(electron_id,electron);
					walker_global_times.pop_front();
				}

        // reorder the walkers based on which one will move next
        walker_global_times.sort(compareSecondItemOfPair);
      }

			auto clusters = CGsystem.getClusters();
			cout << "Number of clusters " << clusters.size() << endl;

    }// End of the Coarse grain simulation 
    

  } // End of coarse grain Monte Carlo
  high_resolution_clock::time_point coarse_time_end = high_resolution_clock::now();

  auto duraction_crude = duration_cast<milliseconds>(setup_time_end-setup_time_start+crude_time_end-crude_time_start).count();
  auto duraction_coarse = duration_cast<milliseconds>(setup_time_end-setup_time_start+coarse_time_end-coarse_time_start).count();

  cout << "Crude Monte Carlo Run Time: " << duraction_crude << " ms " << endl;
  cout << "Coarse Monte Carlo Run Time: " << duraction_coarse << " ms " << endl;
  return 0;
}
