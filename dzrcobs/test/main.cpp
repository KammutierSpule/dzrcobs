#include <stdlib.h>
#include <time.h>
#include <CppUTest/CommandLineTestRunner.h>

int main(int argc, char** argv)
{
	srand(time(NULL));
  return RUN_ALL_TESTS(argc, argv);
}