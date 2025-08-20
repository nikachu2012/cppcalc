@.str = private unnamed_addr constant [5 x i8] c"KUSA\00", align 1

declare i32 @puts(i8*)

define i32 @main() {
    call i32 @puts(ptr @.str)
    ret i32 0
}

