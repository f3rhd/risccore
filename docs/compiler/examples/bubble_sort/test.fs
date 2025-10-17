func bubble_sort(int[] array, int size) -> void {
    int i = 0;
    for(i...<size;i++){
        int j = 0;
        int swapped = 0;
        for(j...<size-i-1;j++){
            if(array[j] > array[j+1]){
                int temp = array[j+1];
                array[j+1] = array[j];
                array[j] = temp;
                swapped = 1;
            }
        }
        if(swapped == 0){
            return;
        }
    }
}
func is_sorted(int[] array,int size) -> int {
    int i  = 0;
    int is_sorted = 1;
    for(i...<size-1;i++){
        if(array[i] > array[i+1]){
            is_sorted = 0;
            return is_sorted;
        }
    }
    return is_sorted;
}
func main() -> void {
    int[] test_array = {5,4,3,2,1};
    bubble_sort(test_array,5);
    int is_sorted_flag = is_sorted(test_array,5);
    if(is_sorted_flag == 1){
        while(1){

        }
    }
    
}
