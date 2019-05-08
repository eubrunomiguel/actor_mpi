//
//  main.swift
//  patchDist
//
//  Created by Alexander Pöppl on 17.09.18.
//  Copyright © 2018 Alexander Pöppl. All rights reserved.
//

import Foundation

/*let xSize = 7
let ySize = 9

let places = 11*/

let xSize = 33
let ySize = 33
let places = 128


var arr = [[Int]](repeating: [Int](repeating:0, count: xSize), count:ySize)
let arrSize = ySize * xSize
let targetPatchesPerPlace = arrSize / places
let leftoverPlaces = arrSize % places

print("targetPatchesPerPlace=\(targetPatchesPerPlace)")
print("leftoverPlaces=\(leftoverPlaces)")
gcd(xSize, ySize)
let optimalSize = Double(places)/Double(gcd(xSize, ySize))

let xSplits = xSize / Int((Double(xSize) / Double(places).squareRoot()).rounded(.down))
let xBS = xSize / xSplits

let ySplits = (places/xSplits)
let yBS = ySize / ySplits
print ("size: y \(ySize) x \(xSize) splits: y \(ySplits) x \(xSplits)")


let genBase = 50
let iterations = 500
let disparityWeight = 3
let cutWeight = 2

putInitial(arrIn:&arr, ySize:ySize, xSize:xSize, ySplits:ySplits, xSplits:xSplits, places:places)

printArray(xSize: xSize, ySize: ySize, arr: arr, places: places)
var x = createNCopies(arr: &arr, n: 4 * genBase)

var previousBest = 0

for it in 0 ..< iterations {
    let (bestFitness, bestIdx) = performIteration(iteration: it, arrays: &x, ySize: ySize, xSize: xSize, places: places, disparityWeight: disparityWeight, cutWeight: cutWeight, genBase: genBase, previousBest:previousBest)
    previousBest = bestIdx
    bestFitness
}

for i in 0 ..< 4 * genBase {
    let fitnessVal = fitness(disparity:getDisparity(arr: &x[i], places:places), disparityWeight:cutWeight, cut:getEdgeCut(arrIn: &x[i], ySize: ySize, xSize: xSize), cutWeight:disparityWeight)
    print("\(i) edge cut is \(getEdgeCut(arrIn: &x[i], ySize: ySize, xSize: xSize)) and disparity is \(getDisparity(arr: &x[i], places:places)) fitness \(fitnessVal)")
}

var best = Int.max
var bestIdx = -1


for i in 0 ..< 4 * genBase {
    let disparity = getDisparity(arr: &x[i], places:places)
    let edgeCut = getEdgeCut(arrIn: &x[i], ySize: ySize, xSize: xSize)
    let fitnessVal = fitness(disparity:disparity, disparityWeight:disparityWeight, cut:edgeCut, cutWeight:cutWeight)
    if fitnessVal < best {
        best = fitnessVal
        bestIdx = i
    }
}

printArray(xSize: xSize, ySize: ySize, arr: x[bestIdx], places:places)
