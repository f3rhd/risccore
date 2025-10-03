func insertion_sort(int[] array,int size) -> void{
    int i = 1;
    int j = 0;
    int key = 0;

    for(i...<size; i++) {
        key = array[i];
        j = i - 1;

        while(j >= 0 && array[j] > key) {
            array[j+1] = array[j];
            j = j - 1;
        }

       array[j+1] = key;
    }
}
func main() -> void {
    int[5] array = {5,4,3,2,1};
    insertion_sort(array,5);
    /* infinite loop will tell us that the array is sorted */
    if(
        array[0] == 1 && array[1] == 2 && array[2] ==3 && array[3] == 4 && array[4] == 5
    ){
        while(1){

        }
    }
}