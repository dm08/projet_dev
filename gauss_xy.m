harris = load('0651150-6861350-040_terMob2_LAMB93_1.0651150-6861350-040_terMob2_2_LAMB93_0.harris');

x1 = harris(:, 1); 
y1 = harris(:, 2);
x2 = harris(:, 3); 
y2 = harris(:, 4);

dx = x1-x2;
dy = y1-y2;

translation = [dx dy];

taille_matrice = 40;
x = linspace(min(dx),max(dx),taille_matrice);
y = linspace(min(dy),max(dy),taille_matrice);
xy = zeros(taille_matrice,taille_matrice);

for i = 1:length(translation)
    for j = 1:length(x)-1
        for k = 1:length(y)-1
            if translation(i,1)>x(j) && translation(i,1)<=x(j+1)
                if translation(i,2)>y(k) && translation(i,2)<=x(k+1)
                    xy(j,k) = xy(j,k) + 1;
                end
            end
        end%k
    end%j
end%i

surf(xy);
colorbar;

for i=1:size(xy,1)
    for j=1:size(xy,1)
        if xy(i,j)<100
            xy(i,j)=0;
        end
    end
end

xy_max = imregionalmax(xy);
[row,col] = find(xy_max);

row = row - 20;
col = col - 20;

result = [];
for i=1:length(row)
    vect = [row(i);col(i)];
    result = [result,vect];
end%i

affichage = 'La translation (%d,%d) a été détectée.\n';

fprintf(affichage,result);