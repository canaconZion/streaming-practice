#include <gtest/gtest.h>
#include<vector>
#include <unordered_map>
using namespace std;
vector<int> twoSum(vector<int>& nums, int target) {
    std::unordered_map <int,int> map;
    for(int i = 0; i < nums.size(); i++) {
        // 遍历当前元素，并在map中寻找是否有匹配的key
        auto iter = map.find(target - nums[i]); 
        if(iter != map.end()) {
            return {iter->second, i};
        }
        // 如果没找到匹配对，就把访问过的元素和下标加入到map中
        map.insert(pair<int, int>(nums[i], i)); 
    }
    return {};
}
TEST(SumTest,test0){
    int a[4] = { 2,7,11,15 };
	int b[2] = {0,1};
	vector<int> a1(a, a + 4);
    vector<int> b1(b,b+2);
    EXPECT_EQ(twoSum(a1,9),b1);
}
TEST(SumTest,test1){
    int a[3] = { 3,2,4 };
	int b[2] = {1,2};
	vector<int> a1(a, a + 3);
    vector<int> b1(b,b+2);
    EXPECT_EQ(twoSum(a1,6),b1);
}
TEST(SumTest,test2){
    int a[3] = {3,3};
	int b[2] = {0,1};
	vector<int> a1(a, a + 2);
    vector<int> b1(b,b+2);
    EXPECT_EQ(twoSum(a1,6),b1);
}
int main(int argc,char **argv){
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}