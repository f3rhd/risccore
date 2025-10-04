func min(int[] array, int size) -> int {
    int min = array[0];
    int i = 1;
    for(i...<size;i++) {
        if(array[i] < min){
            min = array[i];
        }
    }
    return min;
}
func max(int[] array, int size) -> int {
    int max = 0 ;
    int i = 1;
    for(i...<size;i++) {
        if(array[i] > max){
            max = array[i];
        }
    }
    return max;
}
func main() -> void {
    int[] array = {1,2,3,4,5};
    int min_array = min(array,5);
    int max_array = max(array,5);
    if(min_array == 1 && max_array ==1){
        while(1){

        }
    }
}
