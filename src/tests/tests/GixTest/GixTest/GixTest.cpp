#include <base.h>
#include <mine.h>

int main(int argc, char* argv[]) {
	{
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000);

		for (int i = 0; i < 100000; i++){
			TIntUInt64Pr x(i, i);
			gix.AddItem(x, i*5 % 16);
			if (i % 5000 == 1){
				printf("Evo: %d\n", i);
			}
		}
		
	}
	return 0;
}

