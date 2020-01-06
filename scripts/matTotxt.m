function [] = matTotxt(node,path)
    b = struct2cell(node);
    bif_index = find(sum(squeeze(strcmp(b,'bif'))));
    coord = [node(bif_index(:)).coord];
    coord = reshape(coord,3, length(coord)/3)';
    dlmwrite(path,coord);
end
