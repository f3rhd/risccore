func fac(int n) -> int {
    if( n <= 1){
        return n;
    }
    return n*fac(n-1);
}
func main() -> void {
    int x = fac(5);
    /* infinite loop */
    if(x==120){
        while(1){
        }
    }
}