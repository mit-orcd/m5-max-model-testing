int max_subarray(const int *a, int n) {
    int max_sum = a[0];
    int current_sum = a[0];
    
    for (int i = 1;class Solution {
public:
    int maxSubArray(vector<int>& nums) {
        int max_sum = nums[0];
        int current_sum = nums[0];
        
        for (int i = 1; i < nums.size(); i++) {
            current_sum = (current_sum > 0) ? current_sum + nums[i] : nums[i];
            max_sum = (max_sum > current_sum) ? max_sum : current_sum;
        }
        
        return max_sum;
    }
};