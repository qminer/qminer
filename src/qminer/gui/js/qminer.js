function StoreCtrl($scope) {
    $scope.stores = [ ];
    
    // get list of stores from qminer
    $.ajax({
        url: '../qm_stores',
        success: function (data) { 
            angular.forEach(data, function (store) {
                $scope.stores.push(store)
            });
        }
    });    
}