class ServerSentryVehicleRepairAction : ServerSentryVehicleModActionHandler
{
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto descriptor = new ServerSentryLiveModActionDescriptorDto();
        descriptor.id = "vehicle.repair";
        descriptor.targetType = "vehicle";
        descriptor.label = "Repair";
        descriptor.icon = "toolbox";
        descriptor.description = "Repairs the vehicle, restores missing parts and fills all fluids.";
        descriptor.AddScope("live");
        return descriptor;
    }

    override bool ExecuteVehicle(ServerSentryModActionContext ctx, Transport vehicle, out string message)
    {
        if (!vehicle)
        {
            message = "No valid vehicle was selected.";
            return false;
        }

        int spawnedParts;
        int failedParts;

        RestoreMissingAttachments(vehicle, spawnedParts, failedParts);
        RepairEntity(vehicle);

        int repairedParts = RepairAttachmentsRecursive(vehicle);
        int filledReservoirs = FillVehicleFluids(vehicle);

        CarScript car;
        if (Class.CastTo(car, vehicle))
        {
            car.ForceUpdateLightsStart();
            car.ForceUpdateLightsEnd();
        }

        message = string.Format("Vehicle repaired: %1 part(s) repaired, %2 missing part(s) spawned, %3 fluid reservoir(s) filled, %4 part(s) unavailable.", repairedParts, spawnedParts, filledReservoirs, failedParts);
        return true;
    }

    protected void RepairEntity(EntityAI entity)
    {
        if (!entity)
            return;

        entity.SetHealth01("", "Health", 1.0);

        TStringArray damageZones = new TStringArray();
        entity.GetDamageZones(damageZones);

        foreach (string damageZone : damageZones)
            entity.SetHealth01(damageZone, "Health", 1.0);

        ComponentEnergyManager energyManager = entity.GetCompEM();
        if (energyManager)
            energyManager.SetEnergy(energyManager.GetEnergyMax());

        ItemBase item = ItemBase.Cast(entity);
        if (item && item.HasQuantity() && item.GetQuantityMax() > 0)
            item.SetQuantity(item.GetQuantityMax());
    }

    protected int RepairAttachmentsRecursive(EntityAI parent)
    {
        if (!parent || !parent.GetInventory())
            return 0;

        int repairedCount;
        int attachmentCount = parent.GetInventory().AttachmentCount();

        for (int i = 0; i < attachmentCount; i++)
        {
            EntityAI attachment = parent.GetInventory().GetAttachmentFromIndex(i);

            if (!attachment)
                continue;

            RepairEntity(attachment);
            repairedCount++;
            repairedCount += RepairAttachmentsRecursive(attachment);
        }

        return repairedCount;
    }

    protected void RestoreMissingAttachments(Transport vehicle, out int spawnedCount, out int failedCount)
    {
        spawnedCount = 0;
        failedCount = 0;

        if (!vehicle || !vehicle.GetInventory())
            return;

        string attachmentConfigPath = "CfgVehicles " + vehicle.GetType() + " attachments";

        if (!GetGame().ConfigIsExisting(attachmentConfigPath))
            return;

        TStringArray attachmentSlots = new TStringArray();
        GetGame().ConfigGetTextArray(attachmentConfigPath, attachmentSlots);

        foreach (string slotName : attachmentSlots)
        {
            if (slotName == "")
                continue;

            int slotId = InventorySlots.GetSlotIdFromString(slotName);

            if (slotId == InventorySlots.INVALID)
                continue;

            if (vehicle.GetInventory().FindAttachment(slotId))
                continue;

            EntityAI attachment = CreateCompatibleAttachment(vehicle, slotName, slotId);

            if (attachment)
            {
                RepairEntity(attachment);
                spawnedCount++;
            }
            else
            {
                failedCount++;
                Print("[ServerSentry] Could not restore attachment slot '" + slotName + "' on vehicle '" + vehicle.GetType() + "'.");
            }
        }
    }

    protected EntityAI CreateCompatibleAttachment(Transport vehicle, string requiredSlotName, int requiredSlotId)
    {
        string configRoot = "CfgVehicles";
        int configClassCount = GetGame().ConfigGetChildrenCount(configRoot);

        for (int i = 0; i < configClassCount; i++)
        {
            string candidateType;
            GetGame().ConfigGetChildName(configRoot, i, candidateType);

            if (candidateType == "")
                continue;

            string candidatePath = configRoot + " " + candidateType;

            if (GetGame().ConfigGetInt(candidatePath + " scope") != 2)
                continue;

            string inventorySlotPath = candidatePath + " inventorySlot";

            if (!GetGame().ConfigIsExisting(inventorySlotPath))
                continue;

            TStringArray candidateSlots = new TStringArray();
            GetGame().ConfigGetTextArray(inventorySlotPath, candidateSlots);

            if (candidateSlots.Find(requiredSlotName) == -1)
                continue;

            EntityAI attachment = vehicle.GetInventory().CreateAttachmentEx(candidateType, requiredSlotId);

            if (attachment)
                return attachment;
        }

        return null;
    }

    protected int FillVehicleFluids(Transport vehicle)
    {
        CarScript car;
        if (Class.CastTo(car, vehicle))
            return FillCarFluids(car);

        BoatScript boat;
        if (Class.CastTo(boat, vehicle))
            return FillBoatFluids(boat);

        return 0;
    }

    protected int FillCarFluids(CarScript car)
    {
        if (!car)
            return 0;

        int filledCount;
        filledCount += FillCarFluid(car, CarFluid.FUEL);
        filledCount += FillCarFluid(car, CarFluid.OIL);
        filledCount += FillCarFluid(car, CarFluid.BRAKE);
        filledCount += FillCarFluid(car, CarFluid.COOLANT);
        return filledCount;
    }

    protected int FillCarFluid(CarScript car, int fluidType)
    {
        float capacity = car.GetFluidCapacity(fluidType);

        if (capacity <= 0)
            return 0;

        car.Fill(fluidType, capacity);
        return 1;
    }

    protected int FillBoatFluids(BoatScript boat)
    {
        if (!boat)
            return 0;

        float capacity = boat.GetFluidCapacity(BoatFluid.FUEL);

        if (capacity <= 0)
            return 0;

        boat.Fill(BoatFluid.FUEL, capacity);
        return 1;
    }
}