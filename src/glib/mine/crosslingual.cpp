/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
namespace TCrossLingual
{	
	void TCLPairwiseProjector::Load(const TStr& ModelFolder_) {
		// Important:
		// export folder should contain only folders for language pairs
		// each folder corresponds to a pair of projectors, the folder name should be a string "Lang1Id_Lang2Id"
		TStr ModelFolder = ModelFolder_;
		ModelFolder.ChangeChAll('\\', '/');
		if (ModelFolder[ModelFolder.Len()-1] == '/') {
			ModelFolder = ModelFolder_.GetSubStr(0, ModelFolder.Len()-2);
		}

		TStrV DirNames;
		TStrV Ext(1); Ext[0] = "";
		TFFile::GetFNmV(ModelFolder, Ext, false, DirNames);

		Projectors.Gen(DirNames.Len(), 0);
		Centers.Gen(DirNames.Len(), 0);
		LangIds.Gen(DirNames.Len(), 0);
		SwDispTmMsg("Start loading projectors");
		for (int PairN = 0; PairN < DirNames.Len(); PairN++) {
			printf("%s\n%s\n", DirNames[PairN].CStr(), ModelFolder.CStr());
			//TODO check if DirNames[PairN] is a folder
			TStr PairID = DirNames[PairN].RightOfLast('/');
			printf("%s\n", PairID.CStr());
			TStr Lang1ID;
			TStr Lang2ID;
			PairID.SplitOnCh(Lang1ID, '_', Lang2ID);
			LangIds.Add(TPair<TStr,TStr>(Lang1ID, Lang2ID));
			TFIn CenterFile1(DirNames[PairN] + "/c1.bin");
			TFltV c1(CenterFile1);
			TFIn CenterFile2(DirNames[PairN] + "/c2.bin");
			TFltV c2(CenterFile2);
			Centers.Add(TPair<TFltV,TFltV>(c1, c2));
			TFIn ProjectorFile1(DirNames[PairN] + "/P1.bin");
			TFltVV P1(ProjectorFile1);
			TFIn ProjectorFile2(DirNames[PairN] + "/P2.bin");
			TFltVV P2(ProjectorFile2);
			Projectors.Add(TPair<TFltVV,TFltVV>(P1, P2));

			SwDispTmMsg("Loaded projectors in :" + DirNames[PairN]);
		}
		for (int i = 0; i < DirNames.Len(); i++) {
			printf("%s %s\n", LangIds[i].Val1.CStr(), LangIds[i].Val2.CStr());
		}
	}	
	
	void TCLHubProjector::Load(const TStr& ModelFolder_, const TStr& HubHubProjectorDir) {
		// Important:
		// export folder should contain only folders for language pairs and a folder ConMat
		// each folder corresponds to a pair of projectors, the folder name should be a string "Lang1Id_Lang2Id"
		// ASSUMPTION! : Lang1Id must equal the hub language!
		TStr ModelFolder = ModelFolder_;
		ModelFolder.ChangeChAll('\\', '/');
		if (ModelFolder[ModelFolder.Len()-1] == '/') {
			ModelFolder = ModelFolder_.GetSubStr(0, ModelFolder.Len()-2);
		}

		TStrV DirNames;
		TStrV Ext(1); Ext[0] = "";
		TFFile::GetFNmV(ModelFolder, Ext, false, DirNames);		

		Projectors.Gen(DirNames.Len()-1,0);
		Centers.Gen(DirNames.Len()-1,0);
		InvDoc.Gen(DirNames.Len()-1,0);
		LangIds.Gen(DirNames.Len()-1);
		int PairN = 0;
		for (int DirN = 0; DirN < DirNames.Len(); DirN++) {
			TStr PairID = DirNames[DirN].RightOfLast('/');
			if (PairID.EqI("ConMat")) {				
				continue;
			}

			TStr Lang1ID;			
			TStr Lang2ID;
			PairID.SplitOnCh(Lang1ID, '_', Lang2ID);

			if (PairID.EqI(HubHubProjectorDir)) {
				HubHubProjectorIdx = PairN;
				HubLangId = Lang1ID;
			}
			LangIds.AddDat(TPair<TStr,TStr>(Lang1ID, Lang2ID), PairN);
			
			TFIn Center1File(DirNames[DirN] + "/c1.bin");
			TFltV c1(Center1File);
			TFIn Center2File(DirNames[DirN] + "/c2.bin");
			TFltV c2(Center2File);
			Centers.Add(TPair<TFltV,TFltV>(c1, c2));
			TFIn idoc1In(DirNames[DirN] + "/invdoc1.bin");
			TFltV idoc1(idoc1In);
			TFIn idoc2In(DirNames[DirN] + "/invdoc2.bin");
			TFltV idoc2(idoc2In);
			InvDoc.Add(TPair<TFltV, TFltV>(idoc1, idoc2));
			TFIn Projector1File(DirNames[DirN] + "/P1.bin");
			TFltVV P1(Projector1File);
			TFIn Projector2File(DirNames[DirN] + "/P2.bin");
			TFltVV P2(Projector2File);
			Projectors.Add(TPair<TFltVV,TFltVV>(P1, P2));
			PairN++;
		}

		TStr ConPath = ModelFolder + "/ConMat";		
		Ext.Add("bin");
		TStrV FileNames;
		TFFile::GetFNmV(ConPath, Ext, false, FileNames);
		ConMatrices.Gen(FileNames.Len(),0);
		ConIdxH.Gen(FileNames.Len());
		for (int FileN = 0; FileN < FileNames.Len(); FileN++) {
			TPair<TFltVV, TFltVV> ConMat;
			TFIn File(FileNames[FileN]);
			ConMat.Load(File);			
			ConMatrices.Add(ConMat);
			printf("%s\n", FileNames[FileN].CStr());

			TStr PairID = FileNames[FileN].RightOfLast('/');
			PairID = PairID.LeftOfLast('.');
			printf("%s\n", PairID.CStr());
			TStr Lang1ID;
			TStr Lang2ID;
			PairID.SplitOnCh(Lang1ID, '_', Lang2ID);
			ConIdxH.AddDat(TPair<TStr,TStr>(Lang1ID, Lang2ID), FileN);
		}	
		
	}


	void TCLHubProjector::Load(const TStr& ProjectorPathsDirNm, const TStr& ConMatPathsFNm, const TStr& HubHubProjectorLangPairId) {
		// use linux paths not windows !
		// remove last / !
		
		TStr Path;
		if (!TFile::Exists(ProjectorPathsDirNm)){
			printf("Projectors path file %s does not exist", ProjectorPathsDirNm.CStr());
		}
		TFIn ProjectorPathReader(ProjectorPathsDirNm);
		int ProjPairs = 0;
		while (ProjectorPathReader.GetNextLn(Path)) {
			ProjPairs++;
		}		
		ProjectorPathReader.Reset();
		Projectors.Gen(ProjPairs,0);
		Centers.Gen(ProjPairs,0);
		InvDoc.Gen(ProjPairs,0);
		LangIds.Gen(ProjPairs);
		int PairN = 0;
		while (ProjectorPathReader.GetNextLn(Path)) {
			printf("Projector path %s", Path.CStr());
			if (Path[Path.Len() -1] == '/') {
				Path = Path.GetSubStr(0, Path.Len() - 2);
			}
			TStr PairID = Path.RightOfLast('/');
			TStr Lang1ID;			
			TStr Lang2ID;
			PairID.SplitOnCh(Lang1ID, '_', Lang2ID);
			if (PairID.EqI(HubHubProjectorLangPairId)) {
				HubHubProjectorIdx = PairN;
				HubLangId = Lang1ID;
			}
			LangIds.AddDat(TPair<TStr,TStr>(Lang1ID, Lang2ID), PairN);

			TFIn Center1File(Path + "/c1.bin");
			TFltV c1(Center1File);
			TFIn Center2File(Path + "/c2.bin");
			TFltV c2(Center2File);
			Centers.Add(TPair<TFltV,TFltV>(c1, c2));
			TFIn idoc1In(Path + "/invdoc1.bin");
			TFltV idoc1(idoc1In);
			TFIn idoc2In(Path + "/invdoc2.bin");
			TFltV idoc2(idoc2In);
			InvDoc.Add(TPair<TFltV, TFltV>(idoc1, idoc2));
			TFIn Projector1File(Path + "/P1.bin");
			TFltVV P1(Projector1File);
			TFIn Projector2File(Path + "/P2.bin");
			TFltVV P2(Projector2File);
			Projectors.Add(TPair<TFltVV,TFltVV>(P1, P2));

			PairN++;
			printf("loaded hub projector %s, dims %d %d %d %d %d %d %d %d\n", Path.CStr(), c1.Len(), c2.Len(), idoc1.Len(), idoc2.Len(), P1.GetRows(), P1.GetCols(), P2.GetRows(), P2.GetCols());
		}
		printf("Hubs OK\n");
		TInt ConMatrixCount = 0;
		TFIn ConMatricesPathReader(ConMatPathsFNm);
		while (ConMatricesPathReader.GetNextLn(Path)) {
			ConMatrixCount++;
		}
		ConMatricesPathReader.Reset();	
		ConMatrices.Gen(ConMatrixCount,0);
		ConIdxH.Gen(ConMatrixCount);
		int FileN = 0;
		printf("Starting and reading conmat %s\n", ConMatPathsFNm.CStr());
		while (ConMatricesPathReader.GetNextLn(Path)) {		
			printf("Given path: %s\n", Path.CStr());
			TPair<TFltVV, TFltVV> ConMat;
			//Check if File exists and then proceed with loading
			TStr PairID = Path.RightOfLast('/');
			TStr AlternatePath = Path.LeftOfLast('/');
			TStr Ending = PairID.RightOfLast('.');
			PairID = PairID.LeftOfLast('.');
			printf("%s\n", PairID.CStr());
			TStr Lang1ID;
			TStr Lang2ID;
			PairID.SplitOnCh(Lang1ID, '_', Lang2ID);
			AlternatePath = AlternatePath + "/" + Lang2ID + "_" + Lang1ID + "." + Ending;
			if (!TFile::Exists(Path)){
				if (TFile::Exists(AlternatePath)){
					printf("Using alternate path: %s\n", Path.CStr());
					printf("Swapping language ids!\n");
					TStr Temp = Lang2ID;
					Lang2ID = Lang1ID;
					Lang1ID = Temp;
					Path = AlternatePath;
				}
			}
			TFIn File(Path);
			ConMat.Load(File);			
			ConMatrices.Add(ConMat);
			printf("%s %g %g %d %d %g %g %d %d\n", Path.CStr(), ConMat.Val1(0, 0).Val, ConMat.Val1(0, 1).Val, ConMat.Val1.GetRows(), ConMat.Val1.GetCols(), ConMat.Val2(0, 0).Val, ConMat.Val2(0, 1).Val, ConMat.Val2.GetRows(), ConMat.Val2.GetCols());
			ConIdxH.AddDat(TPair<TStr,TStr>(Lang1ID, Lang2ID), FileN);

			printf("loaded conmat %s\n", Path.CStr());

			FileN++;
			//char c;
			//std::cin >> c;
		}
	}

	// Maps language identifiers (DocLangId, Lang2Id) to Projector, Center, ConMat
	// ASSUMPTION: All Projector data has been loaded and will stay there (references are passed)
	bool TCLHubProjector::SelectMatrices(const TStr& DocLangId, const TStr& Lang2Id, const TFltVV*& ProjMat, const TFltV*& Center, const TFltVV*& ConMat, const TFltV*& InvDocV, const TBool& DmozSpecial) const {
		bool OK = true;
		// Give reference to Projector and Center and ConMat
		//printf("\n\n%s %d\n\n", HubLangId.CStr(), HubHubProjectorIdx);
		//printf ("%d ffekjffoief\n", HubHubProjectorIdx);
		bool DocHub = DocLangId.EqI(HubLangId);
		bool OtherHub = Lang2Id.EqI(HubLangId);
		int ProjIdx;
		
		// Val1
		// hub - hub
		if (DocHub && OtherHub) {
			//printf("DOCHUB OTHERHUB!\n");
			ProjMat = &Projectors[HubHubProjectorIdx].Val1;
			Center = &Centers[HubHubProjectorIdx].Val1;
			InvDocV = &InvDoc[HubHubProjectorIdx].Val1;
		}
		// hub - other
		if (DocHub && !OtherHub) {
			//printf("DocHUb other not hub\n");
			int KeyId;
			if (LangIds.IsKey(TStrPr(DocLangId, Lang2Id), KeyId)) {
				ProjIdx = LangIds[KeyId];
				ProjMat = &Projectors[ProjIdx].Val1;
				//printf("%d\n", ProjMat);
				Center = &Centers[ProjIdx].Val1;
				InvDocV = &InvDoc[ProjIdx].Val1;
			} else {
				OK = false;
				if (!DmozSpecial){
					//printf("ok false in dochub && !otherdocuh. ARE YOU MISSING A PATH IN hubprojectors.txt IN CONFIG FOLDER?\n");
				}
				else{
					//printf("I am assuming identity matrix by default, i hope i am right?\n");
				}
			}
		}
		// Val2
		// other - hub
		if (!DocHub && OtherHub) {
			int KeyId;
			if (LangIds.IsKey(TStrPr(Lang2Id, DocLangId), KeyId)) {
				ProjIdx = LangIds[KeyId];
				ProjMat = &Projectors[ProjIdx].Val2;
				Center = &Centers[ProjIdx].Val2;
				InvDocV = &InvDoc[ProjIdx].Val2;
			}
			else {
				OK = false;
				if (!DmozSpecial){
					//printf("ok false in !dochub && otherdocuh. ARE YOU MISSING A PATH IN hubprojectors.txt IN CONFIG FOLDER?\n");
				}
				else{
					//printf("I am assuming identity matrix by default, i hope i am right?\n");
				}
				
			}
		}
		// other - other
		if (!DocHub && !OtherHub) {
			//printf("!DOCHUB !OTHERHUB!\n");
			int KeyId;
			if (LangIds.IsKey(TStrPr(HubLangId, DocLangId), KeyId)) {
				ProjIdx = LangIds[KeyId];
				ProjMat = &Projectors[ProjIdx].Val2;
				Center = &Centers[ProjIdx].Val2;
				InvDocV = &InvDoc[ProjIdx].Val2;
			} else {
				OK = false;
				printf("ok false in !dochub && !otherdocuh. ARE YOU MISSING A PATH IN hubprojectors.txt IN CONFIG FOLDER?\n");
			}
		}
		// Find DocLangId, Lang2Id in ConMat
		int KeyId;
		if (ConIdxH.IsKey(TStrPr(DocLangId, Lang2Id), KeyId)) {
			ConMat = &ConMatrices[ConIdxH[KeyId]].Val1;
		// Else, find Lang2Id, DocLangId in Conmat
		} else if (ConIdxH.IsKey(TStrPr(Lang2Id, DocLangId), KeyId)) {
			ConMat = &ConMatrices[ConIdxH[KeyId]].Val2;
		} else {
			OK = false;
			//printf("ok false in conmat, ARE YOU MISSING A PATH IN conmat.txt IN CONFIG FOLDER? %d %d\n", DocHub, OtherHub);
		}
		return OK;
	}

	bool TCLHubProjector::GetProxyMatrix(const TStr& DocLangId_, const TStr& Lang2Id_, const TFltVV*& ConMat, bool &transpose_flag, const TStr& Friend) const{
		bool OK = true;
		transpose_flag = false;
		TStr DocLangId = DocLangId_;
		TStr Lang2Id   = Lang2Id_;
		if (DocLangId_.EqI(HubLangId)){
			//Going from en-es, en projected to en-Friend
			//Connect with Friend-es
			DocLangId = Friend;
		}
		if (Lang2Id_.EqI(HubLangId)){
			Lang2Id = Friend;
		}
		//printf("%s, %s\n", DocLangId.CStr(), Lang2Id.CStr());
		//char c;
		//std::cin >> c;
		int KeyId;
		if (ConIdxH.IsKey(TStrPr(DocLangId, Lang2Id), KeyId)) {
			ConMat = &ConMatrices[ConIdxH[KeyId]].Val1;
			transpose_flag = true;
			// Else, find Lang2Id, DocLangId in Conmat
		}
		else if (ConIdxH.IsKey(TStrPr(Lang2Id, DocLangId), KeyId)) {
			ConMat = &ConMatrices[ConIdxH[KeyId]].Val1;
			transpose_flag = false;
		}
		else {
			OK = false;
			//printf("ok false in conmat, ARE YOU MISSING A PATH IN conmat.txt IN CONFIG FOLDER? %d %d\n", DocHub, OtherHub);
		}
		//printf("Proxy matrix size: (%d, %d)\n", (*ConMat).GetRows(), (*ConMat).GetCols());
		return OK;
	}

	//Dmoz special flag is set to true by default ... it omits errors when identity matrices are not needed
	//Beware only use Dmoz special flag in cases Hub - Other, Other - Hub
	void TCLHubProjector::Project(const TPair<TIntV, TFltV>& Doc, const TStr& DocLangId, const TStr& Lang2Id, TFltV& Projected, const TBool& DmozSpecial) const {
		const TFltVV* ProjMat = NULL; 
		const TFltV* Center = NULL;
		const TFltVV* ConMat = NULL;
		const TFltV* InvDocV = NULL;
		bool OK = TCLHubProjector::SelectMatrices(DocLangId, Lang2Id, ProjMat, Center, ConMat, InvDocV, DmozSpecial);
		if (!OK){
			if (DmozSpecial){
				//printf("Dmoz special case, identity matrix assumed!\n");
			}
			else{
				//printf("select failed\n");
			}
		}
		//printf("SELECTY hub projector %s %s, \n", DocLangId.CStr(), Lang2Id.CStr());
		//printf("SELECTY center, invdoc dims %d %d \n",Center->Len(), InvDocV->Len());
		//printf("SELECTY projmat %d %d\n ", ProjMat->GetRows(), ProjMat->GetCols());
		if (ConMat != NULL){}
		//printf("SELECTY conmat dims %d %d \n", ConMat->GetRows(), ConMat->GetCols());
		


		// Reweight Doc!
		TPair<TIntV, TFltV> DocTemp = Doc;
		for (int ElN = 0; ElN < Doc.Val1.Len(); ElN++) {
			DocTemp.Val2[ElN] = Doc.Val2[ElN] * (*InvDocV)[Doc.Val1[ElN]];
			//printf("Original: %d %g\n Weight %g\n After: %g\n", Doc.Val1[ElN], Doc.Val2[ElN], (*InvDocV)[Doc.Val1[ElN]], DocTemp.Val2[ElN]);
		}


		////Projected = ConMat(ProjMat*Doc - Center)
		TFltV Pd(ProjMat->GetCols());
		//printf("Projection start!\n");
#ifdef COLMAJOR_DATA
		TLinAlg::Multiply(*ProjMat, DocTemp, Pd);
#else
		//printf("Row major case projection matrices\n");
		//TFltVV temp = *ProjMat; temp.Transpose();
		//printf("Size of projection matrices: (%d, %d)\n", ProjMat->GetXDim(), ProjMat->GetYDim());
		TLinAlg::MultiplyT(DocTemp, *ProjMat, Pd);
#endif
		//printf("Projection done!\n");
		TFltV Pd_minus_c(Center->Len());
		TLinAlg::AddVec(-1.0, *Center, Pd, Pd_minus_c);
		//printf("Centering done!\n");
		if ( (!DmozSpecial && Projected.Empty()) ) {
			//printf("Generation of projected vector of length %d \n", ConMat->GetRows());
			Projected.Gen(ConMat->GetRows());
			//printf("Generation of projected vector done\n");
		}
		//When DmozSpecial flag is enabled identity matrix is assumed ... no change!!!
		if (!DmozSpecial){
			//printf("Normal multiplication!\n");
#ifdef COLMAJOR_DATA
			TLinAlg::Multiply(*ConMat, Pd_minus_c, Projected);
#else
			TLinAlg::MultiplyT(*ConMat, Pd_minus_c, Projected);
#endif

		}
		else{
			Projected = Pd_minus_c;
		}
		/*printf("Vec\n");
		for (int i = 0; i < 10; i++){
			printf("%g\t", Projected[i].Val);
		}*/
	}
	void TCLHubProjector::DoTfidf(TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TStr& DocLangId, const TStr& Lang2Id){
		const TFltVV* ProjMat = NULL;
		const TFltV* Center = NULL;
		const TFltVV* ConMat = NULL;
		const TFltV* InvDocV = NULL;
		bool OK = TCLHubProjector::SelectMatrices(DocLangId, Lang2Id, ProjMat, Center, ConMat, InvDocV);
		if (!OK) printf("Select failed\n");
		TTmStopWatch time;
		//time.Start();
		for (int ElN = 0; ElN < DocMatrix.Val1.Len(); ElN++) {
			DocMatrix.Val3[ElN] = DocMatrix.Val3[ElN] * (*InvDocV)[DocMatrix.Val1[ElN]];
		}
		//time.Stop("Inverse indexing costs: ");
	}
	void TCLHubProjector::DoTfidf(TPair<TIntV, TFltV>& Doc, const TStr& DocLangId, const TStr& Lang2Id){
		const TFltVV* ProjMat = NULL;
		const TFltV* Center = NULL;
		const TFltVV* ConMat = NULL;
		const TFltV* InvDocV = NULL;
		bool OK = TCLHubProjector::SelectMatrices(DocLangId, Lang2Id, ProjMat, Center, ConMat, InvDocV);
		if (!OK) printf("Select failed\n");
		TTmStopWatch time;
		time.Start();
		for (int ElN = 0; ElN < Doc.Val1.Len(); ElN++) {
			Doc.Val2[ElN] = Doc.Val2[ElN] * (*InvDocV)[Doc.Val1[ElN]];
		}
		time.Stop("Inverse indexing costs: ");
	}

	void TCLHubProjector::Project(const TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TStr& DocLangId, const TStr& Lang2Id, TFltVV& Projected, const TBool& Tfidf, const TBool& DmozSpecial) const {
		const TFltVV* ProjMat = NULL;
		const TFltV* Center = NULL;
		const TFltVV* ConMat = NULL;
		const TFltV* InvDocV = NULL;
		//printf("start select\n");
		bool OK = TCLHubProjector::SelectMatrices(DocLangId, Lang2Id, ProjMat, Center, ConMat, InvDocV);
		if (!OK){
			if (DmozSpecial){
				//printf("Dmoz special case, identity matrix assumed!\n");
			}
			else{
				//printf("select failed\n");
			}
		}
		if (OK){
			//printf("selected hub projector %s %s, dims %d %d %d %d %d %d\n", DocLangId.CStr(), Lang2Id.CStr(), Center->Len(), ConMat->GetRows(), ConMat->GetCols(), InvDocV->Len(), ProjMat->GetRows(), ProjMat->GetCols());
		}
		else{
			//printf("selected hub projector %s %s identity assumed, dims %d %d %d %d %d %d\n", DocLangId.CStr(), Lang2Id.CStr(), Center->Len(), Center->Len(), Center->Len(), InvDocV->Len(), ProjMat->GetRows(), ProjMat->GetCols());
		}

		TTmStopWatch time;
		TTriple<TIntV, TIntV, TFltV> DocMatrixTemp;
		if (Tfidf){
			DocMatrixTemp = DocMatrix;
			//time.Start();
			for (int ElN = 0; ElN < DocMatrixTemp.Val1.Len(); ElN++) {
				DocMatrixTemp.Val3[ElN] = DocMatrix.Val3[ElN] * (*InvDocV)[DocMatrix.Val1[ElN]];
			}
			//time.Stop("Inverse indexing costs: ");
			//time.Reset(false);
		}
		//Projected = ConMat(ProjMat*DocMatrix - Center* ones(1, size(DocMatrix,2)))
		int Docs = DocMatrix.Val2[DocMatrix.Val2.GetMxValN()] + 1;
		//printf("Number of docs %d\n", Docs);
		// Compute: Pd_minus_cones := ProjMat*DocMatrix - Center* ones(1, size(DocMatrix,2))
		TFltVV Pd_minus_cones;
#ifdef COLMAJOR_DATA
		Pd_minus_cones.Gen(ProjMat->GetRows(), Docs);
#else
		Pd_minus_cones.Gen(Docs, ProjMat->GetCols());
#endif
		//printf("Vector generated \n");
		//printf("Start multiply sparse %d", Docs);
		//time.Start();
		//ROW and COLUMN major mess
		//TLinAlg::Transpose(DocMatrixTemp, DocMatrixTempT);
		if (Tfidf){
			//printf("TFidf\n");
#ifdef COLMAJOR_DATA
			//Andrej index template madness, add IndexType to template
			TLinAlg::Multiply(*ProjMat, DocMatrixTemp, Pd_minus_cones);
#else
			//Andrej index template madness, add IndexType to template
			TLinAlg::MultiplyT(DocMatrixTemp, *ProjMat, Pd_minus_cones);
#endif
		}
		else{
#ifdef COLMAJOR_DATA
			//Andrej index template madness, add IndexType to template
			TLinAlg::Multiply(*ProjMat, DocMatrix, Pd_minus_cones);
#else
			//Andrej index template madness, add IndexType to template
			TLinAlg::MultiplyT(DocMatrix, *ProjMat, Pd_minus_cones);
#endif
		}
		//time.Stop("Projection costs: ");
		//time.Reset(false);
		//SwDispTmMsg("finished multiply");
		//time.Start();

#ifdef COLMAJOR_DATA
	#ifndef INTEL
		for (int ColN = 0; ColN < Pd_minus_cones.GetCols(); ColN++) {
			TLinAlg::AddVec(-1.0, *Center, Pd_minus_cones, ColN);
		}
	#else
		TFltV enke(Pd_minus_cones.GetCols()); TLAMisc::Fill(enke, 1);
		for (int RowN = 0; RowN < Pd_minus_cones.GetRows(); RowN++) {
			TFltV row; Pd_minus_cones.GetRowPtr(RowN, row);
			TLinAlg::AddVec((*Center)[RowN].Val, enke, row);
		}
	#endif
#else
	#ifndef INTEL
		for (int RowN = 0; RowN < Pd_minus_cones.GetRows(); RowN++) {
			TFltV row;
			Pd_minus_cones.GetRowPtr(RowN, row);
			TLinAlg::AddVec(-1, *Center, row, row);
			//TLinAlg::AddVec(-1.0, *Center, row);
			//TLinAlg::AddVec(-1.0, *Center, Pd_minus_cones, RowN);
		}
	#else
		for (int RowN = 0; RowN < Pd_minus_cones.GetRows(); RowN++) {
			//printf("%d/%d\n", RowN, Pd_minus_cones.GetRows());
			TFltV row;
			Pd_minus_cones.GetRowPtr(RowN, row);
			TLinAlg::AddVec(-1.0, *Center, row);
		}
	#endif
#endif

		//time.Stop("Naive centering of data: ");
		//time.Reset(false);
		//SwDispTmMsg("finished advec");
		if (Projected.Empty() && !DmozSpecial) {
			printf("Projected is empty!!!");
#ifdef COLMAJOR_DATA
			Projected.Gen(ConMat->GetRows(), Pd_minus_cones.GetCols());
#else
			Projected.Gen(Pd_minus_cones.GetRows(), ConMat->GetCols());
#endif
		}	
		//SwDispTmMsg("finished gen");
		//time.Start();
		if (ConMat != NULL){
			if (DmozSpecial || (*ConMat)(0, 0).Val == 1.0){
				//[ANDREJ]printf("Identity matrix detected!!!\n");
				Projected = Pd_minus_cones;
			}
			else{
#ifdef COLMAJOR_DATA
				TLinAlg::Multiply(*ConMat, Pd_minus_cones, Projected);
#else
				TLinAlg::Multiply(Pd_minus_cones, *ConMat, Projected);
#endif

			}
		}
		else{
			Projected = Pd_minus_cones;
		}
		//time.Stop("Projection with connection matrix costs: ");
		//SwDispTmMsg("finished multiply conmat pd_minus_cones");
	}

	TCLCore::TCLCore(TCLProjector* Projectors_, const TStrV& TokenzierLangIdV, const TStrV& TokenizerPaths) {
		Projectors = Projectors_;
		Tokenizers.Gen(TokenizerPaths.Len());
		for (int TokenizerN = 0; TokenizerN < TokenizerPaths.Len(); TokenizerN++) {
			TUnicodeVSM::PGlibUBow Tokenizer = TUnicodeVSM::PGlibUBow::New();
			TFIn TokenizerFile(TokenizerPaths[TokenizerN]);
			Tokenizer->LoadBin(TokenizerFile);
			Tokenizers.AddDat(TokenzierLangIdV[TokenizerN], Tokenizer);			
		}
	}

	TCLCore::TCLCore(TCLProjector* Projectors_, const TStr& TokenizerPathsFNm) {
		Projectors = Projectors_;
		TFIn TokenizerReader(TokenizerPathsFNm);
		TStrV TokenizerPathV;
		TStr Path;
		while (TokenizerReader.GetNextLn(Path)) {
			// extract 
			TStr LangId = Path.RightOfLast('/');
			LangId = LangId.LeftOf('.');
			TUnicodeVSM::PGlibUBow Tokenizer = TUnicodeVSM::PGlibUBow::New();
			TFIn TokenizerFile(Path);
			Tokenizer->LoadBin(TokenizerFile);
			Tokenizers.AddDat(LangId, Tokenizer);
			printf("langid %s\n", LangId.CStr());
			printf("loaded tokenizer %s\n", Path.CStr());
		}		
	}
	
	void TCLCore::TextToVector(TUStr& Text, const TStr& LangId, TPair<TIntV, TFltV>& SparseVec) {
		if (Tokenizers.IsKey(LangId)){
			Tokenizers.GetDat(LangId)->TextToVec(Text, SparseVec);
		}
	}

	void TCLCore::TextToVector(TUStrV& Docs, const TStr& LangId, TTriple<TIntV, TIntV, TFltV>& DocMatrix) {
		if (Tokenizers.IsKey(LangId)){
			Tokenizers.GetDat(LangId)->TextToVec(Docs, DocMatrix);
		}
	}

	void TCLCore::Project(TUStr& Text, const TStr& DocLangId, const TStr& Lang2Id, TFltV& Projected, const TBool& DmozSpecial) {
		TPair<TIntV, TFltV> SparseVec;
		TextToVector(Text, DocLangId, SparseVec);
		Project(SparseVec, DocLangId, Lang2Id, Projected);
	}

	void TCLCore::Project(const TPair<TIntV, TFltV>& Doc, const TStr& DocLangId, const TStr& Lang2Id, TFltV& Projected, const TBool& DmozSpecial) {
		Projectors->Project(Doc, DocLangId, Lang2Id, Projected, DmozSpecial);
	}
	
	void TCLCore::Project(TUStrV& Docs, const TStr& DocLangId, const TStr& Lang2Id, TFltVV& ProjectedMat, const TBool& DmozSpecial) {
		TTriple<TIntV, TIntV, TFltV> DocMatrix;
		TextToVector(Docs, DocLangId, DocMatrix);
		Projectors->Project(DocMatrix, DocLangId, Lang2Id, ProjectedMat, DmozSpecial);
	}

	void TCLCore::Project(const TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TStr& DocLangId, const TStr& Lang2Id, TFltVV& Projected, const TBool& DmozSpecial) {
		Projectors->Project(DocMatrix, DocLangId, Lang2Id, Projected, DmozSpecial);
	}

	double TCLCore::GetSimilarity(TUStr& Text1, TUStr& Text2, const TStr& Lang1Id, const TStr& Lang2Id) {
		TPair<TIntV, TFltV> Doc1;
		TPair<TIntV, TFltV> Doc2;		
		TextToVector(Text1, Lang1Id, Doc1);
		TextToVector(Text2, Lang2Id, Doc2);
		//printf("Transformation to vectors 2 done!\n");
		if (Doc1.Val1.Len() == 0 || Doc2.Val1.Len() == 0){
			return 0;
		}
		//printf("Transformation to vectors 1 done: %d!\n", Doc1.Val1.Len());
		//printf("Transformation to vectors 2 done: %d!\n", Doc2.Val1.Len());
		//printf("Transformation to vectors done!\n");
		return GetSimilarity(Doc1, Doc2, Lang1Id, Lang2Id);
	}

	double TCLCore::GetSimilarity(const TPair<TIntV, TFltV>& Doc1, const TPair<TIntV, TFltV>& Doc2, const TStr& Lang1Id, const TStr& Lang2Id){
		if (Doc1.Val1.Len() == 0 || Doc2.Val2.Len() == 0){
			return 0;
		}
		TFltV PDoc1; TFltV PDoc2;
		//Project(Doc1, Lang1Id, Lang2Id, PDoc1, false);
		//Project(Doc2, Lang2Id, Lang1Id, PDoc2, false);
		Project(Doc1, Lang1Id, Lang2Id, PDoc1, false);
		Project(Doc2, Lang2Id, Lang1Id, PDoc2, false);
		//printf("Projections done!\n");
		TLinAlg::Normalize(PDoc1);
		//printf("Pdoc1:%g\n", PDoc1[0]);
		TLinAlg::Normalize(PDoc2);
		bool transpose_flag = true;
		const TFltVV* Proxy = NULL;
		this->Projectors->GetProxyMatrix(Lang1Id, Lang2Id, Proxy, transpose_flag, this->Friend);
		//printf("Pdoc2:%g\n", PDoc2[0]);
		return TLinAlg::DotProduct(PDoc1, PDoc2);		
	}

	//Take Care of Memory management inside
	void TCLCore::GetSimilarity(const TTriple<TIntV, TIntV, TFltV>& DocMtx1, const TTriple<TIntV, TIntV, TFltV>& DocMtx2, const TStr& Lang1Id, const TStr& Lang2Id, TFltVV& SimMtx){
		// PDocMtx1 should be transposed to obtain better performance
		TFltVV PDocMtx1;  Project(DocMtx1, Lang1Id, Lang2Id, PDocMtx1);
		//printf("(%d, %d)\n", PDocMtx1.GetXDim(), PDocMtx1.GetYDim());
		TFltVV PDocMtx2;  Project(DocMtx2, Lang2Id, Lang1Id, PDocMtx2);
		//printf("(%d, %d)\n", PDocMtx2.GetXDim(), PDocMtx2.GetYDim());
		TLinAlg::NormalizeColumns(PDocMtx1);
		TLinAlg::NormalizeColumns(PDocMtx2);
		//printf("(%d, %d)\n", SimMtx.GetXDim(), SimMtx.GetYDim());
		return TLinAlg::MultiplyT(PDocMtx1, PDocMtx2, SimMtx);
	}


	// Returns top k words in lang1 and top k words in lang2 that contributed to the similarity
	double TCLCore::ExplainSimilarity(const TPair<TIntV, TFltV>& Doc1, const TPair<TIntV, TFltV>& Doc2, const TStr& Lang1Id, const TStr& Lang2Id, TVec<TPair<TFlt,TInt> >& Lang1Words, TVec<TPair<TFlt,TInt> >& Lang2Words) {
		int n1 = Doc1.Val1.Len();
		int n2 = Doc2.Val1.Len();
		// Transform the Doc into a matrix (each word gets mapped to a weighted indicator vector, a column in the matrix)
		TTriple<TIntV, TIntV, TFltV> Doc1Mat;

		Doc1Mat.Val1.Gen(n1, 0);
		Doc1Mat.Val2.Gen(n1, 0);
		Doc1Mat.Val3.Gen(n1, 0);
		for (int ElN = 0; ElN < n1; ElN++) {
			Doc1Mat.Val1.Add(Doc1.Val1[ElN]);
			Doc1Mat.Val2.Add(ElN);
			//Doc1Mat.Val2.Add(Doc1.Val1[ElN]);
			//Doc1Mat.Val1.Add(ElN);
			Doc1Mat.Val3.Add(Doc1.Val2[ElN]);
		}
		TTriple<TIntV, TIntV, TFltV> Doc2Mat;
		Doc2Mat.Val1.Gen(n2, 0);
		Doc2Mat.Val2.Gen(n2, 0);
		Doc2Mat.Val3.Gen(n2, 0);
		
		for (int ElN = 0; ElN < n2; ElN++) {
			Doc2Mat.Val1.Add(Doc2.Val1[ElN]);
			Doc2Mat.Val2.Add(ElN);
			//Doc2Mat.Val2.Add(Doc2.Val1[ElN]);
			//Doc2Mat.Val1.Add(ElN);
			Doc2Mat.Val3.Add(Doc2.Val2[ElN]);
		}
		//printf("Transformation to matrix done!\n");
		TFltVV PDoc1Mat;
		TFltV PDoc1; 
		TFltVV PDoc2Mat;
		TFltV PDoc2;
		Project(Doc1Mat, Lang1Id, Lang2Id, PDoc1Mat);
		//printf("First 1 one done\n");
		Project(Doc1, Lang1Id, Lang2Id, PDoc1);

		//printf("First 2 one done\n");
		Project(Doc2Mat, Lang2Id, Lang1Id, PDoc2Mat);
		//printf("First 3 one done\n");
		Project(Doc2, Lang2Id, Lang1Id, PDoc2);
		//printf("First 4 one done\n");
		
		TFltV Sim1(n1, n1);
		TFltV Sim2(n2, n2);
		//printf("Reserve done\n");
		//printf("(%d %d) * %d = %d", PDoc1Mat.GetXDim(), PDoc1Mat.GetYDim(), PDoc1.Len(), Sim1.Len());
		//printf("(%d %d) * %d = %d", PDoc2Mat.GetXDim(), PDoc2Mat.GetYDim(), PDoc2.Len(), Sim2.Len());
#ifdef COLMAJOR_DATA
		TLinAlg::MultiplyT(PDoc1Mat, PDoc2, Sim1);
		TLinAlg::MultiplyT(PDoc2Mat, PDoc1, Sim2);
#else
		TLinAlg::Multiply(PDoc1Mat, PDoc2, Sim1);
		TLinAlg::Multiply(PDoc2Mat, PDoc1, Sim2);
#endif
		//printf("Sim1: %d\n", Sim1.Len());
		//printf("Sim1: %d\n", Sim1.Len());
		//printf("Reveal magic\n");
		//printf("%d", Lang1Words.Len());
		Lang1Words.Gen(n1, 0);
		//printf("Alocation magic\n");
		for (int ElN = 0; ElN < n1; ElN++) {			
			Lang1Words.Add(TPair<TFlt, TInt>(Sim1[ElN], Doc1.Val1[ElN]));
		}	
		
		Lang2Words.Gen(n2, 0);
		for (int ElN = 0; ElN < n2; ElN++) {
			Lang2Words.Add(TPair<TFlt, TInt>(Sim2[ElN], Doc2.Val1[ElN]));
		}
		
		TLinAlg::Normalize(PDoc1);
		TLinAlg::Normalize(PDoc2);
		//printf("Normalite DOne\n");
		Lang1Words.Sort(false);
		Lang2Words.Sort(false);
		//TLinAlg::DotProduct(PDoc1, PDoc2);
		//printf("Sort Words DOne\n");
		/*
		for (int i = 0; i < PDoc1.Len(); i++){
			printf("%g\t", PDoc1[i].Val);
		}
		printf("\n");
		for (int i = 0; i < PDoc2.Len(); i++){
			printf("%g\t", PDoc2[i].Val);
		}*/
		//printf("\n");
	
		//return 1;
		return TLinAlg::DotProduct(PDoc1,PDoc2);
	}

	
	// Returns top k words as TStr in lang1 and top k words in lang2 that contributed to the similarity
	double TCLCore::ExplainSimilarity(TUStr& Text1, TUStr& Text2, const TStr& Lang1Id, const TStr& Lang2Id, TUStrV& Lang1TopWords, TUStrV& Lang2TopWords, const int& k = 10) {
		TPair<TIntV, TFltV> Doc1;
		TPair<TIntV, TFltV> Doc2;
		TextToVector(Text1, Lang1Id, Doc1);
		TextToVector(Text2, Lang2Id, Doc2);
		printf("Doc1\n");
		for (int i = 0; i < Doc1.Val1.Len(); i++){
			printf("%s: (%d, %g)", GetWordByKeyIdLangId(Lang1Id, Doc1.Val1[i]).GetStr().CStr(), Doc1.Val1[i].Val, Doc1.Val2[i].Val);
		}
		printf("\nDoc2\n");
		for (int i = 0; i < Doc2.Val2.Len(); i++){
			printf("%s: (%d, %g)", GetWordByKeyIdLangId(Lang2Id, Doc2.Val1[i]).GetStr().CStr(), Doc2.Val1[i].Val, Doc2.Val2[i].Val);
		}

		if (Doc1.Val1.Len() == 0 || Doc2.Val1.Len() == 0){
			return 0;
		}
		TVec<TPair<TFlt,TInt> > Lang1Words;
		TVec<TPair<TFlt,TInt> > Lang2Words;
		double sim =  ExplainSimilarity(Doc1, Doc2, Lang1Id, Lang2Id, Lang1Words, Lang2Words);	
		//printf("Sim done!\n");
		//printf("%d %d %f\n", Lang1Words.Len(), Lang2Words.Len(), sim);
		// Copy Lang1Words (vector of pairs (sim,widx)) to Lang1TopWords (string vector)
		Lang1TopWords.Gen(0);		
		//Andrej Macro is probably better, no arguments missmatch
		for (int WordN = 0; WordN < TMath::Mn(k, Lang1Words.Len()); WordN++) {
			Lang1TopWords.Add(GetWordByKeyIdLangId(Lang1Id, Lang1Words[WordN].Val2));
		}
		//printf("Lang1Words %d\n", Lang1Words.Len());
		Lang2TopWords.Gen(0);
		for (int WordN = 0; WordN < TMath::Mn(k, Lang2Words.Len()); WordN++) {
			Lang2TopWords.Add(GetWordByKeyIdLangId(Lang2Id, Lang2Words[WordN].Val2));
		}
		//printf("Lang2Words %d\n", Lang2Words.Len());

		return sim;

	}

	void TCLCNode::Save(TSOut& SOut) const {
		DocIdxV.Save(SOut);
		CatStr.Save(SOut);
		CatPathStr.Save(SOut);
		StrToChildIdH.Save(SOut);
	}
	void TCLCNode::Disp() {printf("cat:%s, catpath:%s, docs:%d, children:%d\n", CatStr.CStr(), CatPathStr.CStr(), DocIdxV.Len(), StrToChildIdH.Len());}

	void TCLClassifier::LoadData() {
		printf("%s\n", (DmozPath + "/DmozCategories.Bin").CStr());
		TFIn CatFile(DmozPath + "/DmozCategories.Bin");
		Cat.Load(CatFile);
		if (CutDepth > -1) {
			for (int DocN = 0; DocN < Cat.Len(); DocN++) {
				TStrV CatParts;
				Cat[DocN].SplitOnAllCh('/', CatParts, true);
				TStr NewCat = "";
				//Andrej
				//int stopN = TMath::Mn(CutDepth, CatParts.Len());
				int stopN = MIN(CutDepth, CatParts.Len());
				for (int k = 0; k < stopN; k++) {
					NewCat += CatParts[k] + "/";
				}
				Cat[DocN] = NewCat;
			}
		}
		if (CutDepth == -2) {
			for (int DocN = 0; DocN < Cat.Len(); DocN++) {
				Cat[DocN] = "Top/" + TInt::GetStr(DocN) + "/";
			}

		}
		printf("%s\n", (DmozPath + "/DmozVectorDocsNew.Bin").CStr());
		TFIn VectorsNewFile(DmozPath + "/DmozVectorDocsNew.Bin");
		Doc.Load(VectorsNewFile);	
		SwDispTmMsg("Done loading dmoz");
	}

	void TCLClassifier::LoadModel(){
		printf("%s\n", (ModelPath + "/Centroids.Bin").CStr());
		//int n; std::cin >> n;
		TFIn CentroidReader(ModelPath + "/Centroids.Bin");
		Centroids.Load(CentroidReader);
		SwDispTmMsg("Number of centroids = " + TInt::GetStr(Centroids.GetRows()) + " " + TInt::GetStr(Centroids.GetCols()));
		TFIn TaxonomyReader(ModelPath + "/Taxonomy.bin");
		Taxonomy.Load(TaxonomyReader);
	}

	void TCLClassifier::ProjectDmoz() {
		//printf("a");
		//printf("%d, %d\n", Doc.Val1[Doc.Val1.GetMxValN()], Doc.Val2[Doc.Val2.GetMxValN()]);
		SwDispTmMsg("Start project");
		CLCore->Project(Doc, HierarchyLangId, Lang2Id, PDoc);
		SwDispTmMsg("Finished project");
		
		TLinAlg::NormalizeColumns(PDoc);
		SwDispTmMsg("Normalization complete");
		//printf("%s\n", SaveFNm.CStr());
		//TFOut File(SaveFNm);
		//PDoc.Save(File);
		//SwDispTmMsg("Saved PDoc");
	}	

	void TCLClassifier::ComputeModel() {
		// TVec<TTriple<TInt, TIntV, TVal> > NodeV; // (ParentNodeId, ChildNodeIdV, NodeVal)
		SwDispTmMsg("Start building tree");
		TCLCNode Root;		
		Root.CatStr = "Top";
		Root.CatPathStr = "Top/";
		Taxonomy.AddRoot(Root);

		for (int DocN = 0; DocN < Cat.Len(); DocN++) {
			TStrV Path;
			Cat[DocN].SplitOnAllCh('/', Path, true);
			TStr CatPartial = "";
			int NodeId = 0;
			for (int PathNodeN = 0; PathNodeN < Path.Len(); PathNodeN++) {
				CatPartial += Path[PathNodeN] + "/";
				// find or create TCLCNode -> use/update: Taxonomy.GetNodeVal(NodeId).StrToChildIdH
				if (PathNodeN == 0) continue; //skip root 
				int KeyId = Taxonomy.GetNodeVal(NodeId).StrToChildIdH.GetKeyId(Path[PathNodeN]);
				if (KeyId >= 0) { // go in
					NodeId = Taxonomy.GetNodeVal(NodeId).StrToChildIdH[KeyId];					
				} else {
					// add
					TCLCNode NodeVal;
					NodeVal.CatPathStr = CatPartial;
					NodeVal.CatStr = Path[PathNodeN];	
					int ParentId = NodeId;
					NodeId = Taxonomy.AddNode(ParentId, NodeVal);
					// add to parent's StrToChildIdH: Path[PathNodeN], NodeId
					Taxonomy.GetNodeVal(ParentId).StrToChildIdH.AddDat(Path[PathNodeN], NodeId);
				}
				// update NodeId
				if (PathNodeN == Path.Len() -1) {
					// add DocN to TCLCNode
					//Taxonomy.GetNodeVal(NodeId).CatStr = Path[PathNodeN];
					Taxonomy.GetNodeVal(NodeId).DocIdxV.Add(DocN);
				}
			}			
		}
		SwDispTmMsg("Finished building tree");		
		TFOut TaxonomyWriter(ModelPath + "/Taxonomy.bin");
		Taxonomy.Save(TaxonomyWriter);			
		SwDispTmMsg("Finished saving tree with " + TInt::GetStr(Taxonomy.GetNodes()) + " nodes.");

		// Statistics: number of documents per cat
		//TreeStats();		
		//bfs
		TIntV IdxV;
		TreeBFS(Taxonomy, IdxV);
		//PrintTreeBFS(Taxonomy, IdxV, ModelPath + "/TreeBFS.txt");		
		IdxV.Reverse();
		//PrintTreeBFS(Taxonomy, IdxV, ModelPath + "/TreeReverseBFS.txt");

		// Centroid at node = mean( mean(data(node)), {centroids of children})
		Centroids.Gen(PDoc.GetRows(), Taxonomy.GetNodes());
		for (int CatN = 0; CatN < Taxonomy.GetNodes(); CatN++) {
			int NodeId = IdxV[CatN];
			//Andrej
			//index NodeId = IdxV[CatN];
			int Docs = Taxonomy.GetNodeVal(NodeId).DocIdxV.Len();
			int Rows = PDoc.GetRows();
			if (Docs > 0) {
				// internal centroid: sum
				for (int DocN = 0; DocN < Docs; DocN++) {
					for (int RowN = 0; RowN < Rows; RowN++) {
						Centroids.At(RowN, NodeId) +=  PDoc.At(RowN, Taxonomy.GetNodeVal(NodeId).DocIdxV[DocN]);
					}
				}
				// internal centroid: average
				TLinAlg::NormalizeColumn(Centroids, NodeId);
			}
			/*for (int RowN = 0; RowN < Rows; RowN++) {
			Centroids.At(RowN, NodeId) /= (double)Docs;
			}*/
			// average internal centroid and centroids of children
			int Children = Taxonomy.GetChildren(NodeId);
			if (Children == 0) continue;
			// sum and normalize
			for (int ChildN = 0; ChildN < Children; ChildN++) {
				int ChildId = Taxonomy.GetChildNodeId(NodeId, ChildN);
				for (int RowN = 0; RowN < Rows; RowN++) {
					Centroids.At(RowN, NodeId) += Centroids.At(RowN, ChildId);
				}
			}
			TLinAlg::NormalizeColumn(Centroids, NodeId);
		}
		SwDispTmMsg("Finished computing centroids");		
		TFOut CentroidWriter(ModelPath + "/Centroids.bin");
		Centroids.Save(CentroidWriter);
		SwDispTmMsg("Finished saving (binary) centroids");	
			
	}

	void TCLClassifier::Classify(TUStr& Text, TInt& Class, const TStr& TextLangId, const TStr& Lang2Id) {
		TPair<TIntV, TFltV> SparseVec;
		CLCore->TextToVector(Text, TextLangId, SparseVec);
		TFltV ProjVec;
		CLCore->Project(SparseVec, TextLangId, Lang2Id, ProjVec);		
		ClassifyProjected(ProjVec, Class);
	}

	// Classify a single document, return top k classes
	void TCLClassifier::Classify(TUStr& Text, TIntV& Class, const TStr& TextLangId, const TStr& Lang2Id, const int& k, const TBool& DmozSpecial) {
		//int kclip = TMath::Mn(k, Centroids.GetCols());
		//Andrej
		int kclip = MIN(k, Centroids.GetCols());
		TPair<TIntV, TFltV> SparseVec;
		//printf("Text to vector start\n");
		CLCore->TextToVector(Text, TextLangId, SparseVec);
		//printf("Text to vector %d finish\n", SparseVec.Val1.Len());
		TFltV ProjVec;
		//printf("Project start\n");
		CLCore->Project(SparseVec, TextLangId, Lang2Id, ProjVec, DmozSpecial);	
		//printf("Project finish %d \n", ProjVec.Len());
		//printf("Classify Projected\n");
		ClassifyProjected(ProjVec, Class, kclip);
		//printf("Classify Projected end\n");
	}
	/*void TCLClassifier::Classify(TFltV& ProjVec, TIntV& Class, const TStr& TextLangId, const TStr& Lang2Id, const int& k, const TBool& DmozSpecial) {
		int kclip = TMath::Mn(k, Centroids.GetCols());
		printf("Classify Projected\n");
		ClassifyProjected(ProjVec, Class, kclip);
		printf("Classify Projected end\n");
	}*/

	void TCLClassifier::ClassifyProjected(const TFltV& ProjVec, TInt& Class) {			
		//SwDispTmMsg("Start classification");
		TFltV Similarities;
		//printf("%d %d dims\n", Centroids.GetCols(), ProjVec.Len());
		Similarities.Gen(Centroids.GetCols());
		//printf("lala\n");
		// return index of maximum Centroids' * ProjVec
		//SwDispTmMsg("Start multiply: Centroids' * ProjVec, dims = " + TInt::GetStr(Centroids.GetRows()) + " " + TInt::GetStr(Centroids.GetCols()) + " " + TInt::GetStr(ProjVec.Len()));
		TLinAlg::MultiplyT(Centroids, ProjVec, Similarities);
		//printf("bleb\n");
		Class = Similarities.GetMxValN();
		//SwDispTmMsg("Finished classification");
	}	
	//Add TFltVV ProjVecVV
	void TCLClassifier::ClassifyProjected(const TFltV& ProjVec, TIntV& Class, const int& k) {
		//Andrej
		//int kclip = TMath::Mn(k, Centroids.GetCols()); 
		int kclip = MIN(k, Centroids.GetCols()); 
		//SwDispTmMsg("Start classification");
		TFltV Similarities;
		Similarities.Gen(Centroids.GetCols());
		// return index of maximum Centroids' * ProjVec
		//SwDispTmMsg("Start multiply: Centroids' * ProjVec, dims = " + TInt::GetStr(Centroids.GetRows()) + " " + TInt::GetStr(Centroids.GetCols()) + " " + TInt::GetStr(ProjVec.Len()));
		TLinAlg::MultiplyT(Centroids, ProjVec, Similarities);
		TVec<TPair<TFlt,TInt> > AugSimilarities(Similarities.Len());
		for (int ElN = 0; ElN < Similarities.Len(); ElN++) {
			AugSimilarities[ElN].Val1 = Similarities[ElN];
			AugSimilarities[ElN].Val2 = ElN;
		}
		AugSimilarities.Sort(false);
		Class.Gen(kclip);
		for (int ElN = 0; ElN < kclip ; ElN++) {
			Class[ElN] = AugSimilarities[ElN].Val2;
		}
		//SwDispTmMsg("Finished classification");
	}

	void TCLClassifier::ClassifyProjected(const TFltVV& ProjVec, TVec<TIntV>& ClassVV, const int& k) {
		int kclip = TMath::Mn(k, Centroids.GetCols());
		//SwDispTmMsg("Start classification");
		TFltVV Similarities;
		Similarities.Gen(ProjVec.GetRows(), Centroids.GetCols());
		// return index of maximum Centroids' * ProjVec
		//SwDispTmMsg("Start multiply: Centroids' * ProjVec, dims = " + TInt::GetStr(Centroids.GetRows()) + " " + TInt::GetStr(Centroids.GetCols()) + " " + TInt::GetStr(ProjVec.Len()));
		TLinAlg::Multiply(ProjVec, Centroids, Similarities);
		TVVec<TPair<TFlt, TInt> > AugSimilarities(Similarities.GetXDim(), Similarities.GetYDim());
		for (int i = 0; i < Similarities.GetXDim(); i++){
			TVec<TPair<TFlt, TInt> > SimPair; AugSimilarities.GetRowPtr(i, SimPair);
			TFltV SimV; Similarities.GetRowPtr(i, SimV);

			for (int ElN = 0; ElN < SimV.Len(); ElN++) {
				SimPair[ElN].Val1 = SimV[ElN];
				SimPair[ElN].Val2 = ElN;
			}
		}
#pragma omp parallel for
		for (int i = 0; i < Similarities.GetXDim(); i++){
			TVec<TPair<TFlt, TInt> > SimPair; AugSimilarities.GetRowPtr(i, SimPair);
			SimPair.Sort(false);
		}
		ClassVV.Gen(Similarities.GetXDim(), Similarities.GetXDim());
#pragma omp parallel for
		for (int i = 0; i < Similarities.GetXDim(); i++){
			TIntV& ClassV = ClassVV[i];
			ClassV.Gen(kclip);
			TVec<TPair<TFlt, TInt> > SimPair; AugSimilarities.GetRowPtr(i, SimPair);
			for (int ElN = 0; ElN < kclip; ElN++) {
				ClassV[ElN] = SimPair[ElN].Val2;
			}
		}
		//SwDispTmMsg("Finished classification");
	}


	void TCLClassifier::GetBestKWordV(const TStrV& PathV, TStrV& KeywordV, TIntV& KeyFqV) {
		THash<TStr, TInt> KwH;
		for (int PathN = 0; PathN < PathV.Len(); PathN++) {
			TStrV Parts;
			PathV[PathN].SplitOnAllCh('/', Parts, true);
			for (int PartN = 0; PartN < Parts.Len(); PartN++) {
				if (KwH.IsKey(Parts[PartN])) {
					KwH.GetDat(Parts[PartN])++;
				}
				else {
					KwH.AddDat(Parts[PartN], 1);
				}
			}
		}
		KwH.SortByDat(false);
		KwH.GetKeyV(KeywordV);
		KwH.GetDatV(KeyFqV);
	}

	void TCLClassifier::GetBestKWordV(const TIntV& ClassV, TStrV& KeywordV, TIntV& KeyFqV) {
		TStrV PathV;
		for (int ElN = 0; ElN < ClassV.Len(); ElN++) {
			TStr Path = GetClassPathStr(ClassV[ElN]);
			TStrV Words; Path.SplitOnAllCh('_', Words, true);
			for (int WordN = 0; WordN < Words.Len(); WordN++) {
				PathV.Add(Words[WordN].ToLc());
			}
		}
		GetBestKWordV(PathV, KeywordV, KeyFqV);
	}

	void TCLClassifier::GetBestKWordV(const TIntV& ClassV, TStrV& KeywordV, TIntV& KeyFqV, TStrV& Categories) {
		TStrV PathV;
		for (int ElN = 0; ElN < ClassV.Len(); ElN++) {
			TStr Path = GetClassPathStr(ClassV[ElN]);
			Categories.Add(Path);
			TStrV Words; Path.SplitOnAllCh('_', Words, true);
			for (int WordN = 0; WordN < Words.Len(); WordN++) {
				PathV.Add(Words[WordN].ToLc());
			}
		}
		GetBestKWordV(PathV, KeywordV, KeyFqV);
	}

	void TCLClassifier::GetBestKWordV(const TVec<TIntV> & ClassVV, TVec<TStrV>& KeywordVV, TVec<TIntV> & KeyFqVV, TVec<TStrV>& CategoriesVV) {
		int n_samples = ClassVV.Len();
		if (KeywordVV.Empty() || KeywordVV.Len() != n_samples){
			KeywordVV.Gen(n_samples, n_samples);
		}
		if (KeyFqVV.Empty() || KeyFqVV.Len() != n_samples){
			KeyFqVV.Gen(n_samples, n_samples);
		}
		if (CategoriesVV.Empty() || CategoriesVV.Len() != n_samples){
			CategoriesVV.Gen(n_samples, n_samples);
		}

#pragma omp parallel for
		for (int i = 0; i < ClassVV.Len(); i++){
			const TIntV& ClassV = ClassVV[i];
			TStrV& KeywordV = KeywordVV[i];
			TIntV& KeyFqV = KeyFqVV[i];
			TStrV& CategoriesV = CategoriesVV[i];
			GetBestKWordV(ClassV, KeywordV, KeyFqV, CategoriesV);
		}
	}


	void TCLClassifier::PrintTreeBFS(TTree<TCLCNode>& Tree, const TIntV& IdxV, const TStr& FNm) {
		TFOut FOut(FNm);
		for (int NodeN = 0; NodeN < IdxV.Len(); NodeN++) {	
			/*if (NodeN < 30) {
			Tree.GetNodeVal(IdxV[NodeN]).Disp();
			}*/
			FOut.PutStr(Tree.GetNodeVal(IdxV[NodeN]).CatPathStr + "\n");
		}
	}
	

}
