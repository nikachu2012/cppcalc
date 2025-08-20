; ModuleID = 'main'
source_filename = "main"

@0 = private unnamed_addr constant [11 x i8] c"Factorial\0A\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@2 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

declare i32 @print(ptr)

declare i32 @printInt(i64)

define i64 @fact_r(i64 %0) {
entry:
  %1 = icmp sgt i64 %0, 0
  br i1 %1, label %then, label %else

then:                                             ; preds = %entry
  %2 = sub i64 %0, 1
  %3 = call i64 @fact_r(i64 %2)
  %4 = mul i64 %0, %3
  br label %end

else:                                             ; preds = %entry
  br label %end

end:                                              ; preds = %else, %then
  %.0 = phi i64 [ %4, %then ], [ 1, %else ]
  ret i64 %.0
}

define i64 @fact_l(i64 %0) {
entry:
  br label %cond

cond:                                             ; preds = %body, %entry
  %.01 = phi i64 [ 1, %entry ], [ %2, %body ]
  %.0 = phi i64 [ 1, %entry ], [ %3, %body ]
  %1 = icmp sle i64 %.0, %0
  br i1 %1, label %body, label %end

body:                                             ; preds = %cond
  %2 = mul i64 %.01, %.0
  %3 = add i64 %.0, 1
  br label %cond

end:                                              ; preds = %cond
  ret i64 %.01
}

define i64 @main() {
entry:
  %0 = call i32 @print(ptr @0)
  %1 = call i64 @fact_r(i64 10)
  %2 = call i32 @printInt(i64 %1)
  %3 = call i32 @print(ptr @1)
  %4 = call i64 @fact_l(i64 10)
  %5 = call i32 @printInt(i64 %4)
  %6 = call i32 @print(ptr @2)
  ret i64 0
}
