#include "Core.h"
#include "UnCore.h"

#include "UnObject.h"
#include "UnMaterial.h"
#include "UnMaterial3.h"

#include "Exporters.h"


void ExportMaterial(const UUnrealMaterial *Mat)
{
	guard(ExportMaterial);

#if RENDERING				// requires UUnrealMaterial::GetParams()

	if (!Mat) return;

	TArray<UUnrealMaterial*> AllTextures;
	Mat->AppendReferencedTextures(AllTextures, false);

	CMaterialParams Params;
	Mat->GetParams(Params);
	if ((Params.IsNull() || Params.Diffuse == Mat) && AllTextures.Num() == 0)
	{
		// empty/unknown material, or material itself is a texture
		appPrintf("Ignoring %s'%s' due to empty parameters\n", Mat->GetClassName(), Mat->Name);
		return;
	}

	FArchive* Ar = CreateExportArchive(Mat, FAO_TextFile, "%s.mat", Mat->Name);
	if (!Ar) return;

#define PROC(Arg)	\
	if (Params.Arg) \
	{				\
		Ar->Printf(#Arg"=%s\n", Params.Arg->Name); \
		ExportObject(Params.Arg); \
	}

	PROC(Diffuse);
	PROC(Normal);
	PROC(Specular);
	PROC(SpecPower);
	PROC(Opacity);
	PROC(Emissive);
	PROC(Cube);
	PROC(Mask);

#if 0
	// collect all textures - already exported ones and everything else
	TArray<UUnrealMaterial*> ExportedTextures;
	Params.AppendAllTextures(ExportedTextures);
	// now, export only thise which weren't exported yet
	int numOtherTextures = 0;
	for (int i = 0; i < AllTextures.Num(); i++)
	{
		UUnrealMaterial* Tex = AllTextures[i];
		if (ExportedTextures.FindItem(Tex) < 0)
		{
			Ar->Printf("Other[%d]=%s\n", numOtherTextures++, Tex->Name);
			ExportObject(Tex);
		}
	}
#else
	// Dump material properties to a separate file
	FArchive* PropAr = CreateExportArchive(Mat, FAO_TextFile, "%s.props.txt", Mat->Name);
	if (PropAr)
	{
		Mat->GetTypeinfo()->SaveProps(Mat, *PropAr);
		delete PropAr;
	}
#endif

	if (Mat->IsA("MaterialInstanceConstant"))
	{
		const UMaterialInstanceConstant* Inst = static_cast<const UMaterialInstanceConstant*>(Mat);
		if (Inst->Parent)
		{
			ExportMaterial(Inst->Parent);
		}
	}

	delete Ar;

#endif // RENDERING

	unguardf("%s'%s'", Mat->GetClassName(), Mat->Name);
}
