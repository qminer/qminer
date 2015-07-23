/////////////////////////////////////////////////
// Community detection algorithms
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
namespace TSnap {

  namespace TSnapDetail {
  } // namespace TSnapDetail

  TFltV GetTsFeatures(std::string line, TStrV features, int N){

		// variables for features
		int number_of_zeros, number_of_ones, num_of_zero_to_one = 0, num_of_one_to_zero = 0, first_class, last_class, min_zero_area = 0, max_zero_area = 0, min_one_area = 0, max_one_area = 0, 
			period = 0, num_of_zero_areas, num_of_one_areas, one_area=0, zero_area=0;
		double avg_zero_area = 0, avg_one_area = 0, avg_zero_area_rate = 0, avg_one_area_rate = 0, zero_ratio = 0, one_ratio = 0, number_of_one_area_rate = 0, number_of_zero_area_rate = 0, \
			one_to_zero_num_areas_rate = 0, one_rate_1_2 = 0.0, one_rate_2_2 = 0.0, one_rate_1_3 = 0.0, one_rate_2_3 = 0.0, one_rate_3_3 = 0.0, one_rate_1_4 = 0.0, one_rate_2_4 = 0.0, 
			one_rate_3_4 = 0.0, one_rate_4_4 = 0.0, one_rate_1_5 = 0.0, one_rate_2_5 = 0.0, one_rate_3_5 = 0.0, one_rate_4_5 = 0.0, one_rate_5_5 = 0.0, one_rate_1_6 = 0.0, one_rate_2_6 = 0.0, 
			one_rate_3_6 = 0.0, one_rate_4_6 = 0.0, one_rate_5_6 = 0.0, one_rate_6_6 = 0.0, one_balance_1_6 = 0.0, one_balance_2_6 = 0.0, one_balance_3_6 = 0.0, one_balance_4_6 = 0.0, one_balance_5_6 = 0.0, one_balance_6_6 = 0.0, balance_entropy=0.0;

		// print the headers to the console
		/*fprintf(F1, "id\tnumber_of_zeros\tnumber_of_ones\tzero_ratio\tone_ratio\tnum_of_zero_areas\tnum_of_one_areas\tnumber_of_zero_area_rate\tnumber_of_one_area_rate\tone_to_zero_num_areas_rate\t"
			"num_of_zero_to_one\tnum_of_one_to_zero\tmin_zero_area\tmax_zero_area\tavg_zero_area\tavg_zero_area_rate\tmin_one_area\tmax_one_area\tavg_one_area\tavg_one_area_rate\tfirst_class\tlast_class\tone_rate_1_2\t"
			"one_rate_2_2\tone_rate_1_3\tone_rate_2_3\tone_rate_3_3\tone_rate_1_4\tone_rate_2_4\tone_rate_3_4\tone_rate_4_4\tone_rate_1_5\tone_rate_2_5\tone_rate_3_5\tone_rate_4_5\tone_rate_5_5\tone_rate_1_6\tone_rate_2_6\t"
			"one_rate_3_6\tone_rate_4_6\tone_rate_5_6\tone_rate_6_6\tone_balance_1_6\tone_balance_2_6\tone_balance_3_6\tone_balance_4_6\tone_balance_5_6\tone_balance_6_6\tbalance_entropy\n");*/
	
		// initialize the variables
		one_area=0; zero_area=0; number_of_zeros = 0; number_of_ones = 0; num_of_zero_to_one = 0; num_of_one_to_zero = 0; num_of_zero_areas = 0; num_of_one_areas = 0; number_of_one_area_rate = 0; number_of_zero_area_rate = 0; one_to_zero_num_areas_rate = 0;
		avg_zero_area_rate = 0; avg_one_area_rate = 0; first_class = -1; last_class =-1; min_zero_area = 99999999; max_zero_area = -1; min_one_area = 99999999; max_one_area = -1; one_rate_1_2 = 0.0; one_rate_2_2 = 0.0; one_rate_1_3 = 0.0;
		one_rate_2_3 = 0.0; one_rate_3_3 = 0.0; one_rate_1_4 = 0.0; one_rate_2_4 = 0.0; one_rate_3_4 = 0.0; one_rate_4_4 = 0.0; one_rate_1_5 = 0.0; one_rate_2_5 = 0.0; one_rate_3_5 = 0.0; one_rate_4_5 = 0.0; one_rate_5_5 = 0.0; one_rate_1_6 = 0.0;
		one_rate_2_6 = 0.0; one_rate_3_6 = 0.0; one_rate_4_6 = 0.0; one_rate_5_6 = 0.0; one_rate_6_6 = 0.0; one_balance_1_6 = 0.0; one_balance_2_6 = 0.0; one_balance_3_6 = 0.0; one_balance_4_6 = 0.0; one_balance_5_6 = 0.0; one_balance_6_6 = 0.0; balance_entropy=0.0;

		// input time series data
		TIntIntHH out_arr;
		out_arr.AddDat(1);
		out_arr.AddDat(0);
		out_arr.AddDat(1);
		out_arr.AddDat(0);
		out_arr.AddDat(0);
		out_arr.AddDat(1);
		out_arr.AddDat(0);

		// open output file
		//std::ifstream file(OutFNm.CStr());
		// line string for input file lines
		//std::string line;
		std::stringstream linestream(line);
		std::string data;
		std::getline(linestream, data, '\t');

		// temporal variables for data reading
		int val1 = -1;
		int val2 = -1;
		bool first = false;
		double half = (double)((N)/2);
		double third = ((half*2)/3);
		double quarter = half/2;
		double fifth = ((half*2)/5);
		double sixt = (third/2);
		double c_1_2 = 0, c_2_2 = 0;
		double c_1_3 = 0, c_2_3=0, c_3_3 = 0;
		double c_1_4 = 0, c_2_4 = 0, c_3_4 = 0, c_4_4 = 0;
		double c_1_5 = 0, c_2_5 = 0, c_3_5 = 0, c_4_5 = 0, c_5_5 = 0;
		double c_1_6 = 0, c_2_6 = 0, c_3_6 = 0, c_4_6 = 0, c_5_6 = 0, c_6_6 = 0;

		// reading verticaly by years
		for (int i=0; i < N; i++){
			linestream >> val1;
			if (val1 == 0){ number_of_zeros++; zero_area++; }
			if (val1 == 1){ number_of_ones++; one_area++; }
			if (val1 == 0 && val2 == 1){ num_of_one_to_zero++; num_of_one_areas++; if (one_area > max_one_area) max_one_area = one_area; if (one_area < min_one_area) min_one_area = one_area; one_area=0; }
			if (val1 == 1 && val2 == 0){ num_of_zero_to_one++; num_of_zero_areas++; if (zero_area > max_zero_area) max_zero_area = zero_area; if (zero_area < min_zero_area) min_zero_area = zero_area; zero_area=0; }
			if (val1 == -1 && val2 == 0){ num_of_zero_areas++; if (zero_area > max_zero_area) max_zero_area = zero_area; if (zero_area < min_zero_area) min_zero_area = zero_area; zero_area=0; }
			if (val1 == -1 && val2 == 1){ num_of_one_areas++; if (one_area > max_one_area) max_one_area = one_area; if (one_area < min_one_area) min_one_area = one_area; one_area=0; }
			if (val1!=-1 && first == false){ first_class = val1; first = true; }
			if (i==out_arr.Len()-1) last_class = val1;
			if (half > 0){ if (i<half){ if (val1 == 1) one_rate_1_2+=1; c_1_2 +=1; } else{ if (val1 == 1) one_rate_2_2+=1; c_2_2 +=1; } }
			if (third > 0){ 
				if (i<third){ if (val1 == 1) one_rate_1_3+=1; c_1_3 +=1; }
				else if (i < (third*2)) {	if (val1 == 1) one_rate_2_3+=1; c_2_3 +=1; }
				else{ if (val1 == 1) one_rate_3_3+=1; c_3_3 +=1; }	
			}
			if (quarter > 0){
				if (i<quarter){ if (val1 == 1) one_rate_1_4+=1; c_1_4 +=1; }
				else if (i < (quarter*2)){ if (val1 == 1) one_rate_2_4+=1; c_2_4 +=1; }
				else if (i < (quarter*3)){ if (val1 == 1) one_rate_3_4+=1; c_3_4 +=1; }
				else{ if (val1 == 1) one_rate_4_4+=1; c_4_4 +=1; }
			}
			if (fifth>0){ 
				if (i<fifth){ if (val1 == 1) one_rate_1_5+=1; c_1_5 +=1; } else if (i < (fifth*2)){ if (val1 == 1) one_rate_2_5+=1; c_2_5 +=1; }
				else if (i < (fifth*3)){ if (val1 == 1) one_rate_3_5+=1; c_3_5 +=1; }
				else if (i < (fifth*4)){ if (val1 == 1) one_rate_4_5+=1; c_4_5 +=1; }
				else{ if (val1 == 1) one_rate_5_5+=1; c_5_5 +=1; }
			}
			if (sixt >0){
				if (i<sixt){ if (val1 == 1) one_rate_1_6+=1; c_1_6 +=1; }
				else if (i < (sixt*2)){ if (val1 == 1) one_rate_2_6+=1; c_2_6 +=1; }
				else if (i < (sixt*3)){ if (val1 == 1) one_rate_3_6+=1; c_3_6 +=1; }
				else if (i < (sixt*4)){ if (val1 == 1) one_rate_4_6+=1; c_4_6 +=1; }
				else if (i < (sixt*5)){ if (val1 == 1) one_rate_5_6+=1; c_5_6 +=1; }
				else{ if (val1 == 1) one_rate_6_6+=1; c_6_6 +=1; }
			}
			val2 = val1;
		}

		// corrections and calculations (summation, etc.)
		period = number_of_zeros + number_of_ones;
		if (last_class == 0){ num_of_zero_areas++; if (zero_area > max_zero_area) max_zero_area = zero_area; if (zero_area < min_zero_area) min_zero_area = zero_area; }
		else if (last_class == 1){ num_of_one_areas++; if (one_area > max_one_area) max_one_area = one_area; if (one_area < min_one_area) min_one_area = one_area; }\
		if (min_zero_area == 99999999) min_zero_area = 0;
		if (min_one_area == 99999999) min_one_area = 0;	
		if (num_of_zero_areas>0) avg_zero_area = (double)number_of_zeros/(double)num_of_zero_areas;
		if (num_of_one_areas>0) avg_one_area = (double)number_of_ones/(double)num_of_one_areas;
		zero_ratio = (double)number_of_zeros/(double)period;
		one_ratio = (double)number_of_ones/(double)period;
		number_of_one_area_rate = (double)num_of_one_areas/((double)period/2);
		number_of_zero_area_rate = (double)num_of_zero_areas/((double)period/2);
		if (number_of_one_area_rate+number_of_zero_area_rate>0) one_to_zero_num_areas_rate = number_of_one_area_rate/(number_of_one_area_rate+number_of_zero_area_rate);
		avg_zero_area_rate = avg_zero_area/period;
		avg_one_area_rate = avg_one_area/period;
		if (half >0){ one_rate_1_2 = one_rate_1_2/(double)c_1_2; one_rate_2_2=one_rate_2_2/(double)c_2_2; }
		if (third > 0){ one_rate_1_3=one_rate_1_3/(double)c_1_3; one_rate_2_3=one_rate_2_3/(double)c_2_3; one_rate_3_3=one_rate_3_3/(double)c_3_3; }
		if (quarter> 0){ one_rate_1_4=one_rate_1_4/(double)c_1_4; one_rate_2_4=one_rate_2_4/(double)c_2_4; one_rate_3_4=one_rate_3_4/(double)c_3_4; one_rate_4_4=one_rate_4_4/(double)c_4_4; }
		if (fifth>0){ one_rate_1_5=one_rate_1_5/(double)c_1_5; one_rate_2_5=one_rate_2_5/(double)c_2_5; one_rate_3_5=one_rate_3_5/(double)c_3_5; one_rate_4_5=one_rate_4_5/(double)c_4_5; one_rate_5_5=one_rate_5_5/(double)c_5_5; }
		if (sixt>0){ one_balance_1_6 = one_rate_1_6/(double)number_of_ones;one_balance_2_6 = one_rate_2_6/(double)number_of_ones;one_balance_3_6 = one_rate_3_6/(double)number_of_ones;one_balance_4_6 = one_rate_4_6/(double)number_of_ones;
			one_balance_5_6 = one_rate_5_6/(double)number_of_ones;one_balance_6_6 = one_rate_6_6/(double)number_of_ones; one_rate_1_6=one_rate_1_6/(double)c_1_6; one_rate_2_6=one_rate_2_6/(double)c_2_6; one_rate_3_6=one_rate_3_6/(double)c_3_6;
			one_rate_4_6=one_rate_4_6/(double)c_4_6; one_rate_5_6=one_rate_5_6/(double)c_5_6; one_rate_6_6=one_rate_6_6/(double)c_6_6; }
		balance_entropy = -((one_balance_1_6*log(one_balance_1_6))+(one_balance_2_6*log(one_balance_2_6))+(one_balance_3_6*log(one_balance_3_6))+(one_balance_4_6*log(one_balance_4_6))+(one_balance_5_6*log(one_balance_5_6))+(one_balance_6_6*log(one_balance_6_6)));

		// hash with computed values
		TStrFltH flth;
		flth.AddDat("zero_ratio", zero_ratio); flth.AddDat("one_ratio", one_ratio); flth.AddDat("number_of_ones", number_of_ones); flth.AddDat("number_of_zeros", number_of_zeros); flth.AddDat("one_area",one_area);
		flth.AddDat("zero_area",zero_area); flth.AddDat("num_of_zero_to_one",num_of_zero_to_one); flth.AddDat("num_of_one_to_zero",num_of_one_to_zero); flth.AddDat("num_of_zero_areas",num_of_zero_areas); 
		flth.AddDat("num_of_one_areas",num_of_one_areas); flth.AddDat("number_of_one_area_rate",number_of_one_area_rate);  flth.AddDat("number_of_zero_area_rate",number_of_zero_area_rate); 
		flth.AddDat("one_to_zero_num_areas_rate",one_to_zero_num_areas_rate); flth.AddDat("avg_zero_area_rate",avg_zero_area_rate); flth.AddDat("avg_one_area_rate",avg_one_area_rate); 
		flth.AddDat("first_class",first_class); flth.AddDat("last_class",last_class); flth.AddDat("min_zero_area",min_zero_area); flth.AddDat("max_zero_area",max_zero_area); flth.AddDat("avg_zero_area",avg_zero_area);
		flth.AddDat("min_one_area",min_one_area); flth.AddDat("max_one_area",max_one_area); flth.AddDat("avg_one_area",avg_one_area); flth.AddDat("one_rate_1_2",one_rate_1_2); flth.AddDat("one_rate_2_2",one_rate_2_2); flth.AddDat("one_rate_1_3",one_rate_1_3);
		flth.AddDat("one_rate_2_3",one_rate_2_3); flth.AddDat("one_rate_3_3",one_rate_3_3); flth.AddDat("one_rate_1_4",one_rate_1_4); flth.AddDat("one_rate_2_4",one_rate_2_4); flth.AddDat("one_rate_3_4",one_rate_3_4);
		flth.AddDat("one_rate_4_4",one_rate_4_4); flth.AddDat("one_rate_1_5",one_rate_1_5); flth.AddDat("one_rate_2_5",one_rate_2_5);
		flth.AddDat("one_rate_3_5",one_rate_3_5); flth.AddDat("one_rate_4_5",one_rate_4_5); flth.AddDat("one_rate_5_5",one_rate_5_5); flth.AddDat("one_rate_1_6",one_rate_1_6); flth.AddDat("one_rate_2_6",one_rate_2_6); flth.AddDat("one_rate_3_6",one_rate_3_6);
		flth.AddDat("one_rate_2_3",one_rate_2_3); flth.AddDat("one_rate_3_3",one_rate_3_3); flth.AddDat("one_rate_1_4",one_rate_1_4); flth.AddDat("one_rate_2_4",one_rate_2_4); flth.AddDat("one_rate_3_4",one_rate_3_4);
		flth.AddDat("one_rate_4_6",one_rate_4_6); flth.AddDat("one_rate_5_6",one_rate_5_6); flth.AddDat("one_rate_6_6",one_rate_6_6);
		flth.AddDat("one_balance_1_6",one_balance_1_6); flth.AddDat("one_balance_2_6",one_balance_2_6); flth.AddDat("one_balance_3_6",one_balance_3_6); flth.AddDat("one_balance_4_6",one_balance_4_6); flth.AddDat("one_balance_5_6",one_balance_5_6);
		flth.AddDat("one_balance_6_6",one_balance_6_6); flth.AddDat("balance_entropy", balance_entropy);

		//output
		TFltV outfltv;
		for (int i=0; i<features.Len(); i++){
			TStr name = features[i];
			double d = flth.GetDat(features[i]);
			outfltv.Add(flth.GetDat(features[i]));
		}

		return outfltv;
  }

  void EgonetPropEvolutionBatch(TStr InFNm, TIntFltVH &nn, TIntFltVH &ne, TIntFltVH &edi, TIntFltVH &adi, TIntFltVH &dc, TIntFltVH &bc, TIntFltVH &cc, TIntFltVH &de, TIntFltVH &wcc, TIntV &ids, TIntStrH &idmap, TIntIntVH &years, int &minyear,int &maxyear){
    // declaring variable for the id of the ego node (needed for some properties computation)
    int ego_id;
    // additional variables for properties computation
    TIntFltH centr; TCnComV CnComV;

    // find all files with extension .edg
    HANDLE hFind;
    WIN32_FIND_DATA data;
    char * folder = new char[strlen(InFNm.CStr())+strlen("*.edg")];
    sprintf(folder,"%s%s",InFNm.CStr(),"*.edg");
    hFind = FindFirstFile(folder, &data);

    if (hFind != INVALID_HANDLE_VALUE) {

	// counter for each file, i.e individual
	int internal_id_counter=0;

	// repeat this for each file with .edg extension in the input folder 
	do {

		// declaring temporal vectors
		TFltV nnV, neV, ediV, adiV, dcV, bcV, ccV, deV, wccV;

		// declaring container for years
		TIntV year;

		printf("%s\n", data.cFileName);
		// WORK

		// get the new file to work with
		char * newfile = new char[strlen(InFNm.CStr())+strlen(data.cFileName)];
		sprintf(newfile,"%s%s",InFNm.CStr(),data.cFileName);
		// print the file name to work with
		// printf("%s\n", newfile);

		// subtract the name
		std::string full(data.cFileName);
		int lastindex = full.find_last_of("."); 

		// string of the id is subtracted from the filename
		std::string idstring = full.substr(0, lastindex);

		// ego_id is the id converted to int
		ego_id = atoi(idstring.c_str());

		// DO THE EGONET STATS

		// needed for the existing plotting
		//PGStatVec GStatVec = TGStatVec::New(tmuYear, TGStat::BasicStat());
		
		// parse the file, dont ignore '#' sign because it is used for delimiting networks inside a file
		TSsParser Ss(newfile, ssfWhiteSep, true, false, true);

		// defining variables for node ids and starting year
		int SrcNId, DstNId, t=1970;

		// variable for delimiter between networks
		TStr Marker;

		// go to next line of the file
		Ss.Next();

		// counter for each year 
		int internal_year_counter = 0;

		while (!Ss.Eof()) {
			
			//printf("%i\n", t);
			Marker = Ss.GetLnStr();
			// get the year from the network seperator
			t = Marker.GetSubStr(1,4).GetInt();

			if (Marker.GetCh(0) == '#'){
				Ss.Next();
				PNGraph Graph = PNGraph::TObj::New();
				do{
					if (! Ss.GetInt(0, SrcNId) || ! Ss.GetInt(1, DstNId)) {
						if (!Ss.Eof()){ 
							Ss.Next(); 
							if (!Ss.Eof()) 
								Marker = Ss.GetLnStr();
						}
						continue; 
					}
					if (! Graph->IsNode(SrcNId)) { Graph->AddNode(SrcNId); }
					if (! Graph->IsNode(DstNId)) { Graph->AddNode(DstNId); }
					Graph->AddEdge(SrcNId, DstNId);
					Ss.Next();
					if (!Ss.Eof())
						Marker = Ss.GetLnStr();
				}while(Marker.GetCh(0)!='#' && !Ss.Eof());

				// when true a new network inside of a file is encountered
				if (Graph->GetNodes()>0){
					
					// declaring temporal variables for properties
					TFlt nn_, ne_, edi_, adi_, dc_, bc_, cc_, de_, wcc_;

					// convert the graph into the sutabale format
					Graph->Defrag();
					PUNGraph UGraph = TSnap::ConvertGraph<PUNGraph>(Graph);
					
					// calculate number of nodes in the current graph
					nn_ = Graph->GetNodes();
					// add number of nodes to the vector of number of nodes of the current individual
					nnV.Add(nn_);

					// calucalte number of edges in the current graph
					ne_ = Graph->GetEdges();
					// add the value to the vector for the current individual
					neV.Add(ne_);

					// calculate effective diameter
					TIntFltKdV DistNbrsV;
					TSnap::GetAnf(Graph, DistNbrsV, -1, false, 32);
					edi_= TSnap::TSnapDetail::CalcEffDiam(DistNbrsV, 0.9);
					ediV.Add(edi_);
					
					// calcualte average diameter
					TSnap::GetAnf(Graph, DistNbrsV, -1, false, 32);
					adi_ = TSnap::TSnapDetail::CalcAvgDiamPdf(DistNbrsV);
					adiV.Add(t,adi_);

					// calcualte degree centrality
					dc_ = TSnap::GetDegreeCentr(UGraph,ego_id);
					dcV.Add(dc_);

					// calculate closeness centrality
					double clc = TSnap::GetClosenessCentr(UGraph, ego_id);

					// calculate betweennesss centrality
					TSnap::GetBetweennessCentr(UGraph, centr, 1.0);
					bc_ = centr.GetDat(ego_id);
					bcV.Add(bc_);

					// calculate clustering coefficient
					cc_ = TSnap::GetClustCf(UGraph);
					ccV.Add(cc_);

					// calculate density
					de_ = ne_/(((nn_*nn_)-nn_)/2);
					deV.Add(de_);

					// calculate number of weekly connected components when the ego node is deleted
					Graph->DelNode(ego_id);
					// caluclate if number of edges is greater than 0 after deleting the ego node
					if (Graph->GetEdges()>0){ 
						TSnap::GetWccs(Graph, CnComV);
						wcc_ = CnComV.Len();
						wccV.Add(wcc_);
					}
					else
						wccV.Add(0);

					// add year
					year.Add(t);
					// check the year period range
					if (t>maxyear) maxyear = t;
					if (t<minyear) minyear = t;
				}
				// in case we want to add 0 values for the (empty) years with empty networks
				else{
					nnV.Add(0); neV.Add(0); ediV.Add(0); adiV.Add(0); dcV.Add(0);
					bcV.Add(0); ccV.Add(0); deV.Add(0); wccV.Add(0);
				}
			}
			else Ss.Next();
		}

		// a way which will most likely became the standard is to store each property separetly as a vector (one year peer vector element) 
		// and have a vector for each individual
		// after the vectors for an individual are filled, they are added to the containers of vectors
		
		// depracticated version with key mapping
		/*
		nn.AddDat(internal_id_counter, nnV);
		ne.AddDat(internal_id_counter, neV);
		edi.AddDat(internal_id_counter, ediV);
		adi.AddDat(internal_id_counter, adiV);
		dc.AddDat(internal_id_counter, dcV);
		bc.AddDat(internal_id_counter, bcV);
		cc.AddDat(internal_id_counter, ccV);
		de.AddDat(internal_id_counter, deV);
		wcc.AddDat(internal_id_counter, wccV);
		*/

		nn.AddDat(ego_id, nnV);
		ne.AddDat(ego_id, neV);
		edi.AddDat(ego_id, ediV);
		adi.AddDat(ego_id, adiV);
		dc.AddDat(ego_id, dcV);
		bc.AddDat(ego_id, bcV);
		cc.AddDat(ego_id, ccV);
		de.AddDat(ego_id, deV);
		wcc.AddDat(ego_id, wccV);

		// add years for the current individual
		years.AddDat(internal_id_counter, year);

		// add internal id to the vector
		// depraciticated version with generic ids
		//ids.Add(internal_id_counter);
		ids.Add(ego_id);

		// connect the internal id with the real id
		// depracticated version
		// idmap.AddDat(internal_id_counter, idstring.c_str());
		// mapping not really needed since the id is not generic anymore
		idmap.AddDat(ego_id, idstring.c_str());

		// increase the internal id counter
		internal_id_counter++;

	  } while (FindNextFile(hFind, &data));
	  FindClose(hFind);
    }

  }

}; //namespace TSnap
